
class CValue extends CDefinition {
   constructor() {
      super();
      this.name = null;
      this.type = null;
      this.c_typenames = {
         original:   null,
         serialized: null,
      };
      this.field_info = null;
      this.type_is_signed = null; // pertains to the C type; only relevant for integers
      this.default_value = null;
      this.array_ranks   = []; // array extents
      this.categories    = []; // bitpacking category names
      
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
      
      if (this.type != "transform") { // this.field_info
         let offset = node.getAttribute("c-offset");
         if (offset !== null) {
            let size = node.getAttribute("c-size");
            if (size !== null) {
               this.field_info = {
                  is_bitfield: false,
                  offset:      +offset,
                  size:        +size,
               };
            }
         } else {
            offset = node.getAttribute("c-bitfield-offset");
            if (offset !== null) {
               let size = node.getAttribute("c-bitfield-width");
               if (size !== null) {
                  this.field_info = {
                     is_bitfield: true,
                     offset:      +offset,
                     size:        +size,
                  };
               }
            }
         }
      }
      
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
         } else if (child.nodeName == "category") {
            let name = child.getAttribute("name");
            if (name)
               this.categories.push(name);
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
   
   compute_integer_bounds() {
      console.assert(this.type == "integer");
      const options = this.options;
      let   out = {
         bitcount: options.bitcount,
         min:      options.min,
         max:      options.max,
      };
      
      function _bitmax(bitcount) {
         if (bitcount == 31)
            return 0x80000000;
         if (bitcount == 32)
            return 0xFFFFFFFF;
         //
         // JS bitwise operators coerce operands to int32_t, so the below 
         // expression is improperly signed at a bitcount of 31, and will 
         // produce a result that is not representable at a bitcount of 32.
         //
         return (1 << bitcount) - 1;
      }
      
      let bitcount_max = _bitmax(options.bitcount);
      if (this.type_is_signed && options.min === null && options.max === null) {
         bitcount_max = _bitmax(options.bitcount - 1); // assume a sign bit
         out.min = -bitcount_max;
         out.max = bitcount_max - 1;
      } else if (options.min === null) {
         if (options.max === null) {
            out.min = 0;
            out.max = bitcount_max;
         } else {
            if (out.max > bitcount_max || options.is_signed) {
               out.min = out.max - bitcount_max;
            } else {
               out.min = 0;
            }
         }
      } else if (options.max === null) {
         out.max = min + bitcount_max;
      }
      return out;
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
            this.rebuild_element(i);
         }
      }
   }
   
   get base() { return this.decl; }
   set base(v) {
      console.assert(!v || v instanceof CValue);
      this.decl = v;
   }
   
   get declared_length() { return this.decl.array_ranks[this.rank]; }
   
   rebuild_element(i) {
      let is_innermost = this.rank + 1 == this.decl.array_ranks.length;
      let v;
      if (is_innermost) {
         v = this.decl.make_instance_representation(this.save_format, true);
      } else {
         v = new CValueInstanceArray(this.save_format, this.decl, this.rank + 1);
      }
      v.is_member_of = this;
      this.values[i] = v;
   }
   
   copy_contents_of(/*const CValueInstanceArray*/ other) {
      console.assert(other instanceof CValueInstance);
      console.assert(this.decl.type == other.decl.type);
      console.assert(this.values.length == other.values.length);
      for(let i = 0; i < this.values.length; ++i) {
         this.values[i].copy_contents_of(other.values[i]);
      }
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
   
   // TODO: See if this field name is still used. Replace it if so; then delete the 
   // getter and setter.
   get base() { return this.decl; }
   set base(v) {
      console.assert(!v || v instanceof CValue);
      this.decl = v;
   }
   
   copy_contents_of(/*const CValueInstance*/ other) {
      console.assert(other instanceof CValueInstance);
      console.assert(this.decl.type == other.decl.type);
      if (other.value === null) {
         this.value = null;
         return;
      }
      if (other.value instanceof DataView) {
         const size   = other.value.byteLength;
         let   buffer = new ArrayBuffer(size);
         this.value = new DataView(buffer);
         
         let i;
         for(i = 0; i < size; i += 4)
            this.value.setUint32(other.value.getUint32(i, true), true);
         for(; i < size; ++i)
            this.value.setUint8(other.value.getUint8(i));
         return;
      }
      if (other.value instanceof PokeString) {
         if (!(this.value instanceof PokeString)) {
            this.value = new PokeString();
         }
         this.value.bytes = ([]).concat(other.value.bytes);
         return;
      }
      this.value = other.value;
   }
};