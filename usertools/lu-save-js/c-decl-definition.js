// FIELD_DECL or VAR_DECL.
// Created for declarations of both single values and arrays. For an 
// array, all type information refers to the innermost element type 
// (e.g. for a `int[4][3][2]` field, the type info is for `int`).
class CDeclDefinition extends CDefinition {
   static strip_c_typename(str) {
      if (str === null)
         return null;
      str = str.trim();
      str = str.replace(/^(?:(?:const|volatile) )+/, ""); // strip cv-qualifiers
      str = str.replace(/(?:\[\d+\])+$/, ""); // strip array extents
      str = str.replace(/\*+$/, ""); // strip pointers
      return str;
   }
   
   constructor(/*SaveFormat*/ format) {
      super(format);
      this.name = null; // String
      this.type = null; // enum
      this.c_types = {
         original: {
            name:       null, // String
            definition: null, // CTypeDefinition
         },
         serialized: {
            name:       null, // String
            definition: null, // CTypeDefinition
         },
      };
      this.bitfield_info = null; // FIELD_DECL only
      this.default_value = null;
      this.array_extents = []; // Array<size_t>
      this.categories    = []; // Array<String> // bitpacking categories
      this.annotations   = []; // Array<String> // bitpacking misc annotations
      
      this.options = new CBitpackOptions();
   }
   
   /*Optional<CTypeDefinition>*/ get anonymous_type() {
      if (!this.serialized.name)
         return this.serialized.definition;
   }
   /*CTypeDefinition*/ get serialized_type() {
      return this.c_types.serialized.definition;
   }
   /*String*/ get typename() {
      return this.c_types.serialized.name;
   }
   
   begin_load(node) {
      super.begin_load(node);
      
      this.name = node.getAttribute("name");
      this.type = node.nodeName;
      {  // Load C type info.
         let _load = (function(key, attr) {
            let info = this.c_types[key];
            let name = CDeclDefinition.strip_c_typename(node.getAttribute(attr));
            info.name = name;
            if (name) {
               info.definition = this.save_format.lookup_type_by_name(name);
               assert_xml_validity(!!info.definition, "If a field or variable declaration has a named type, that type must have a corresponding definition.");
            }
         }).bind(this);
         _load("original",   "type");
         _load("serialized", "serialized-type");
         if (!this.c_types.serialized.name) {
            Object.assign(this.c_types.serialized, this.c_types.original);
         }
         if (this.type == "transformed") {
            _load("serialized", "transformed-type");
         }
         //
         // Validate.
         //
         switch (this.type) {
            case "integral":
               assert_xml_validity(this.c_types.serialized.definition instanceof CIntegralTypeDefinition, "If a field or declaration is serialized as an integer, then its serialized type must actually be an integral type.");
               break;
            
            case "union-external-tag":
            case "union-internal-tag":
               assert_xml_validity(this.c_types.serialized.definition instanceof CUnionDefinition, "If a field or declaration is serialized as a union, then its serialized type must actually be a union.");
               break;
         }
      }
      if (this.type == "struct") {
         if (this.c_types.serialized.name) {
            assert_xml_validity(this.c_types.serialized.definition instanceof CStructDefinition, "If a field or declaration is serialized as a struct, then its serialized type must actually be a struct.");
         } else {
            //
            // Handle anonymous structs.
            //
            let type = new CStructDefinition(this.save_format);
            this.c_types.serialized.definition = this.c_types.original.definition = type;
            for(let child of node.children) {
               if (child.nodeName == "members") {
                  for(let item of child.children) {
                     let member = new CDeclDefinition(this.save_format);
                     member.begin_load(item);
                     member.finish_load();
                     type.members.push(member);
                  }
               }
            }
         }
      }
      //
      // Check for bitfield info (only when usable).
      //
      if (this.type != "transformed") {
         let offset = node.getAttribute("c-bitfield-offset");
         let size   = node.getAttribute("c-bitfield-width");
         if (offset !== null && size !== null) {
            this.bitfield_info = {
               is_bitfield: true,
               offset:      +offset,
               size:        +size,
            };
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
      
      this.options.from_xml(node, true);
      
      for(let child of node.children) {
         if (child.nodeName == "array-rank") {
            let extent = +child.getAttribute("extent");
            console.assert(!isNaN(extent));
            this.array_extents.push(extent);
         } else if (child.nodeName == "category") {
            let name = child.getAttribute("name");
            if (name)
               this.categories.push(name);
         } else if (child.nodeName == "annotation") {
            let name = child.getAttribute("text");
            if (name)
               this.annotations.push(name);
         }
      }
   }
   finish_load() {
   }
   
   /*CInstance*/ make_instance_representation(innermost) /*const*/ {
      if (!innermost && this.array_extents.length > 0) {
         return new CArrayInstance(this, 0);
      }
      if (this.type == "union-external-tag" || this.type == "union-internal-tag") {
         return new CUnionInstance(this, this.c_types.serialized.definition);
      }
      if (this.type == "struct") {
         return new CStructInstance(this, this.c_types.serialized.definition);
      }
      if (this.type == "transformed") {
         let type = this.serialized_type;
         if (type instanceof CStructDefinition) {
            return new CStructInstance(this, type);
         }
         if (type instanceof CUnionDefinition) {
            return new CUnionInstance(this, type);
         }
         //
         // In particular, the way we strip C type qualifiers on load will break 
         // for transformations to arrays.
         //
         throw new Error("support for other transformed types is not yet implemented here");
      }
      return new CValueInstance(this);
   }
   
   compute_integer_bounds() {
      assert_logic(this.type == "integer");
      assert_logic(!!this.c_types.serialized.definition);
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
            if (out.max > bitcount_max || this.c_types.serialized.definition.is_signed) {
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