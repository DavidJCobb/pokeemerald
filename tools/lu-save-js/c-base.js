
// Akin to a GCC TYPE node or a decl_descriptor from our codegen plug-in.
class CDefinition {
   constructor(format) {
      assert_logic(format instanceof SaveFormat);
      this.save_format = format;
   }
};

class CTypeDefinition extends CDefinition {
   constructor(format) {
      super(format);
      this.node   = null; // XML node within the SaveFormat
      this.tag    = null; // struct THIS_STRING { ... }
      this.symbol = null; // typedef struct { ... } THIS_STRING
      this.c_info = {
         alignment: null,
         size:      null,
      };
   }
   
   from_xml(node) {
      this.node = node;
   }
};

class CIntegralTypeDefinition extends CTypeDefinition {
   constructor(format) {
      super(format);
      this.is_signed = null;
      
      this.typedefs = [];
   }
   
   from_xml(node) {
      super.from_xml(node);
      this.symbol = node.getAttribute("name");
      
      this.is_signed = node.getAttribute("is-signed") == "true";
      {
         let v = node.getAttribute("alignment");
         if (v !== null)
            this.c_info.alignment = +v;
      }
      {
         let v = node.getAttribute("size");
         if (v !== null)
            this.c_info.size = +v;
      }
      
      for(let child of node.children) {
         if (child.nodeName == "typedef") {
            let name = child.getAttribute("name");
            if (name)
               this.typedefs.push(name);
         }
      }
   }
};

class CContainerTypeDefinition extends CTypeDefinition {
   constructor(format) {
      super(format);
      this.members = []; // Array<CValue>
   }
   
   from_xml(node) {
      super.from_xml(node);
      
      this.symbol = node.getAttribute("name");
      this.tag    = node.getAttribute("tag");
      
      this.c_info.alignment = node.getAttribute("c-alignment");
      this.c_info.size      = node.getAttribute("c-sizeof");
      if (this.c_info.alignment !== null)
         this.c_info.alignment = +this.c_info.alignment;
      if (this.c_info.size !== null)
         this.c_info.size = +this.c_info.size;
   }
   
   /*Optional<CValue>*/ member_by_name(name) /*const*/ {
      for(let memb of this.members)
         if (memb.name == name)
            return memb;
      return null;
   }
};

//
// Instances:
//

class CInstance {
   constructor(/*SaveFormat*/ format) {
      this.save_format  = format;
      
      this.decl = null; // CValue
      this.type = null; // Optional<CTypeDefinition>
      
      this.is_member_of = null; // Optional<CInstance>
   }
   
   // Identifier of the VAR_DECL or FIELD_DECL this CInstance represents.
   get identifier() { return this.decl?.name || null; }
   
   build_path_string() {
      let parts = [this];
      for(let inst = this.is_member_of; inst; inst = inst.is_member_of)
         parts.push(inst);
      parts = parts.reverse();
      
      let path = "";
      for(let i = 0; i < parts.length; ++i) {
         const inst = parts[i];
         const prev = parts[i - 1];
         if (inst instanceof SaveSlot)
            continue;
         
         const is_array_element = prev instanceof CValueInstanceArray;
         if (is_array_element) {
            let index = prev.values.indexOf(inst);
            console.assert(index >= 0);
            path += `[${index}]`;
            continue;
         }
         if (prev && !(prev instanceof SaveSlot)) {
            path += '.';
         }
         let segm = inst.identifier || "__unnamed";
         {
            let deref = inst.decl?.dereference_count;
            if (deref && deref > 0) {
               segm = `(${"".padStart(deref, '*')}${segm})`;
            }
         }
         path += segm;
      }
      return path;
   }
};

class CDeclInstance extends CInstance {
   constructor(/*SaveFormat*/ format, /*CDefinition*/ decl) {
      super(format);
      this.decl = decl;
   }
};
class CTypeInstance extends CInstance {
   constructor(/*SaveFormat*/ format, /*CDefinition*/ type, /*CDefinition*/ decl) {
      super(format);
      this.type = type;
      this.decl = decl;
   }
};