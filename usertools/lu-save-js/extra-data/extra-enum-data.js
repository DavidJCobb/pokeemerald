class ExtraEnumData {
   constructor() {
      this.prefix  = null;
      this.members = {
         by_name:  new Map(), // Map<String, int>
         by_value: new Map(), // Map<int, String>
      };
      this.is_signed     = false;
      this.unused_ranges = []; // Array<Object{ first, last }>
   }
   
   static DisplayOverrides = class DisplayOverrides extends CInstanceDisplayOverrides {
      #enumeration;
      
      constructor(/*ExtraEnumData*/ enumeration) {
         super();
         this.#enumeration = enumeration;
         this.overrides = {
            display_string:      this.#display_string.bind(this),
            make_editor_element: this.#make_editor_element.bind(this),
         };
      }
      
      #display_string(/*CInstance*/ inst) {
         let v = inst.value;
         if (v === null)
            return null;
         let n = this.#enumeration.members.by_value.get(v);
         if (n)
            return "[style=value-text]" + n + "[/style]";
         if (this.unused_ranges) {
            for(let range of this.unused_ranges) {
               if (n >= range.first && n <= range.last) {
                  return "[style=value-text]__UNUSED_" + n + "[/style]";
               }
            }
         }
         return null;
      }
      #make_editor_element(/*CInstance*/ inst) {
         let node = document.createElement("enum-input");
         node.valueNames = this.#enumeration.members.by_name;
         return node;
      }
   };
   
   concat(/*ExtraEnumData*/ other) {
      if (other.prefix != this.prefix)
         return;
      for(const [k, v] of other.members.by_name) {
         this.members.by_name.set(k, v);
         this.members.by_value.set(v, k);
      }
      this.unused_ranges = this.unused_ranges.concat(other.unused_ranges);
   }
   
   make_display_overrides(criteria) {
      let disp = new ExtraEnumData.DisplayOverrides(this);
      if (criteria)
         disp.criteria = criteria;
      return disp;
   }
   
   lookup_name(name) {
      return this.members.by_name.get(name);
   }
   lookup_value(value) {
      return this.members.by_value.get(value);
   }
   
   static peek_enum_name(subrecord_signature, /*Bytestream*/ body) {
      if (subrecord_signature == "ENUMDATA") {
         body.readUint8();
         return body.readLengthPrefixedString(1);
      } else if (subrecord_signature == "ENUNUSED") {
         return body.readLengthPrefixedString(1);
      }
      throw new Error("Unhandled subrecord name " + subrecord_signature + "!");
   }
   
   // Parse enum data from an ENUMDATA subrecord.
   #parse_ENUMDATA(/*Bytestream*/ body) {
      const flags = body.readUint8();
      this.prefix = body.readLengthPrefixedString(1);
      const count = body.readUint32();
      
      this.is_signed   = flags & 1;
      const is_sparse  = flags & 2;
      const has_lowest = flags & 4;
      
      const read_func = "read" + (this.is_signed ? "I" : "Ui") + "nt32";
      
      let names;
      let lowest;
      {
         let block_view;
         let block_size;
         if (is_sparse) {
            block_size = body.readUint32();
            block_view = body.viewFromHere(block_size);
         } else {
            if (has_lowest) {
               lowest = body[read_func]();
            } else {
               lowest = 0;
            }
            block_view = body.viewFromHere();
         }
         let decoder = new TextDecoder();
         names = decoder.decode(block_view).split("\0");
         if (names.length == 1 && !names[0]) {
            names = []; // "" splits to [""] when we'd want []
         }
         if (is_sparse) {
            body.skipBytes(block_size);
         }
         if (names.length != count) {
            throw new Error("Expected " + count + " names; got " + names.length + ".", { cause: names });
         }
      }
      if (is_sparse) {
         for(let i = 0; i < names.length; ++i) {
            //let name  = this.prefix + names[i];
            let name  = names[i];
            let value = body[read_func]();
            this.members.by_name.set(name, value);
            this.members.by_value.set(value, name);
         }
      } else {
         for(let i = 0; i < names.length; ++i) {
            //let name  = this.prefix + names[i];
            let name  = names[i];
            let value = lowest + i;
            this.members.by_name.set(name, value);
            this.members.by_value.set(value, name);
         }
      }
   }
   
   #parse_ENUNUSED(/*Bytestream*/ body) {
      const read_func = "read" + (this.is_signed ? "I" : "Ui") + "nt32";
      {
         let prefix = body.readLengthPrefixedString(1);
         if (this.prefix) {
            if (this.prefix != prefix)
               return;
         } else {
            this.prefix = prefix;
         }
      }
      while (body.remaining) {
         let first = body[read_func]();
         let last  = body[read_func]();
         this.unused_ranges.push({ first: first, last: last });
      }
   }
   
   // Parse data from a subrecord.
   parse(subrecord_signature, /*Bytestream*/ body) {
      switch (subrecord_signature) {
         case "ENUMDATA":
            this.#parse_ENUMDATA(body);
            break;
         case "ENUNUSED":
            this.#parse_ENUNUSED(body);
            break;
      }
   }
};