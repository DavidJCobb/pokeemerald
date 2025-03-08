class ExtraEnumData {
   constructor() {
      this.prefix  = null;
      this.members = {
         by_name:  new Map(), // Map<String, int>
         by_value: new Map(), // Map<int, String>
      };
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
         return null;
      }
      #make_editor_element(/*CInstance*/ inst) {
         let node = document.createElement("enum-input");
         node.valueNames = this.#enumeration.members.by_name;
         return node;
      }
   };
   
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
   
   // Parse enum data from an ENUMDATA subrecord.
   parse(/*Bytestream*/ body) {
      const flags = body.readUint8();
      this.prefix = body.readLengthPrefixedString(1);
      const count = body.readUint32();
      
      const is_signed  = flags & 1;
      const is_sparse  = flags & 2;
      const has_lowest = flags & 4;
      
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
               lowest = body["read" + (is_signed ? "I" : "Ui") + "nt32"]();
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
            let value = body["read" + (is_signed ? "I" : "Ui") + "nt32"]();
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
};