
class CValue extends CDefinition {
   constructor() {
      super();
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
      
      this.options = new CBitpackOptions();
   }
   
   #parse_c_type(str) {
      if (str === null)
         return str;
      str = str.trim();
      str = str.replace(/^(?:(?:const|volatile) )+/, "");
      str = str.replace(/(?:\[\d+\])+$/, ""); // array type to value type
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
      
      if (this.type == "integer") {
         this.type_is_signed = node.getAttribute("type-is-signed") == "true";
      }
      this.options.from_xml(node, true);
      
      for(let child of node.children) {
         if (child.nodeName == "array-rank") {
            let extent = +child.getAttribute("extent");
            console.assert(!isNaN(extent));
            this.array_ranks.push(extent);
         }
      }
      
      if (this.type == "struct" && !this.c_typenames.original) {
         this.anonymous_type = new CStruct();
         for(let child of node.children) {
            if (child.nodeName == "members") {
               this.anonymous_type.members_from_xml(child);
            }
         }
      }
   }
   
   /*CInstance*/ make_instance_representation(save_format, innermost) /*const*/ {
      if (!innermost && this.array_ranks.length > 0) {
         return new CValueInstanceArray(save_format, this, 0);
      }
      if (this.type == "union-external-tag" || this.type == "union-internal-tag") {
         let c_type = save_format.lookup_type_by_name(this.c_typenames.serialized);
         console.assert(!!c_type);
         console.assert(c_type instanceof CUnion);
         return new CUnionInstance(save_format, c_type, this);
      }
      if (this.type == "struct") {
         let c_type = this.anonymous_type;
         if (!c_type) {
            console.assert(!!this.c_typenames.serialized, "struct value must be either an instance of an anonymous struct, or an instance of a named struct");
            c_type = save_format.lookup_type_by_name(this.c_typenames.serialized);
            console.assert(!!c_type, "if a struct value is an instance of a named struct, that struct must exist in the format");
         }
         console.assert(c_type instanceof CStruct);
         return new CStructInstance(save_format, c_type, this);
      }
      return new CValueInstance(save_format, this);
   }
};

// represents a single-dimensional array within the instance tree
class CValueInstanceArray extends CDeclInstance {
   constructor(/*SaveFormat*/ format, /*CValue*/ definition, /*size_t*/ rank) {
      super(format, definition);
      this.values = null;
      this.rank   = rank || 0;
      if (definition) {
         console.assert(definition instanceof CValue);
         if (!rank)
            rank = 0;
         console.assert(rank < definition.array_ranks.length);
         let extent       = definition.array_ranks[rank];
         let is_innermost = rank + 1 == definition.array_ranks.length;
         
         this.values = [];
         for(let i = 0; i < extent; ++i) {
            let v;
            if (is_innermost) {
               v = definition.make_instance_representation(format, true);
            } else {
               v = new CValueInstanceArray(format, definition, rank + 1);
            }
            v.is_member_of = this;
            this.values.push(v);
         }
      }
   }
   
   get base() { return this.decl; }
   set base(v) {
      console.assert(!v || v instanceof CValue);
      this.decl = v;
   }
   
   // Returns a list of any instance-objects that couldn't be filled in.
   fill_in_defaults() {
      if (!this.values)
         return [this];
      
      let unfilled = [];
      for(let member of this.values) {
         if (member instanceof CValueInstance) {
            let dv = member.definition?.default_value;
            if (dv === undefined) {
               unfilled.push(member);
            } else {
               member.value = dv;
            }
            continue;
         }
         unfilled = unfilled.concat(member.fill_in_defaults());
      }
      return unfilled;
   }
};

class CValueInstance extends CDeclInstance {
   constructor(/*SaveFormat*/ format, /*CValue*/ definition) {
      super(format, definition);
      this.value = null;
      if (definition) {
         console.assert(definition instanceof CValue);
         let dv = definition.default_value;
         if (dv !== undefined && dv !== null)
            this.value = dv;
      }
   }
   
   get base() { return this.decl; }
   set base(v) {
      console.assert(!v || v instanceof CValue);
      this.decl = v;
   }
};