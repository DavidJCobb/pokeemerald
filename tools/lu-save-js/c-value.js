
class CValue {
   constructor() {
      this.name = null;
      this.type = null;
      this.c_typenames = {
         original:   null,
         serialized: null,
      };
      this.default_value = null;
      this.array_ranks   = [];
      
      // If this is an anonymous struct, you'll find the type here.
      this.anonymous_type = null;
   }
   
   #parse_c_type(str) {
      if (str === null)
         return str;
      str = str.trim();
      str = str.replace(/^(?:(?:const|volatile) )+/, "");
      str = str.replace(/(?:\[\d+\])+$/, "");
      return str;
   }
   
   from_xml(node) {
      this.name = node.getAttribute("name");
      this.type = node.nodeName;
      this.c_typenames.original   = this.#parse_c_type(node.getAttribute("type"));
      this.c_typenames.serialized = this.#parse_c_type(node.getAttribute("serialized-type"));
      if (!this.c_typenames.serialized)
         this.c_typenames.serialized = this.c_typenames.original;
      
      this.default_value = node.getAttribute("default-value");
      if (this.default_value !== null) {
         switch (this.type) {
            case "integer":
               this.default_value = Math.floor(+this.default_value);
               break;
         }
      } else {
         let dvs = node.querySelector("default-value-string");
         if (dvs)
            this.default_value = dvs.textContent;
      }
      for(let child of node.children)
         if (child.nodeName == "array-rank")
            this.array_ranks.push(+child.getAttribute("extent"));
         
      if (this.type == "struct" && !this.c_typenames.original) {
         this.anonymous_type = new CStruct();
         for(let child of node.children) {
            if (child.nodeName == "members") {
               this.anonymous_type.members_from_xml(child);
            }
         }
      }
   }
};

class CValueInstance {
   constructor(/*SaveFormat*/ format, /*CValue*/ base) {
      this.save_format = format;
      this.base        = base;
      this.value       = null;
      if (base) {
         let instance_of = null;
         if (base.type == "struct") {
            instance_of = base.anonymous_type;
            if (!instance_of) {
               console.assert(!!base.c_typenames.serialized, "struct value must be either an instance of an anonymous struct, or an instance of a named struct");
               instance_of = this.save_format.lookup_type_by_name(base.c_typenames.serialized);
               console.assert(!!instance_of, "if a struct value is an instance of a named struct, that struct must exist in the format");
            }
         }
         
         if (base.array_ranks.length) {
            let recursively_build;
            recursively_build = (function (i) {
               let extent = base.array_ranks[i];
               let out    = [];
               if (i + 1 == base.array_ranks.length) {
                  for(let j = 0; j < extent; ++j) {
                     if (base.default_value) {
                        out[j] = base.default_value;
                     } else if (base.type == "struct") {
                        out[j] = new CStructInstance(this.save_format, instance_of);
                     }
                  }
                  return out;
               }
               for(let j = 0; j < extent; ++j) {
                  out[j] = recursively_build(i + 1);
               }
               return out;
            }).bind(this);
            this.value = recursively_build(0);
         } else if (base.default_value) {
            this.value = base.default_value;
         } else if (instance_of) {
            this.value = new CStructInstance(this.save_format, instance_of);
         }
      }
   }
};