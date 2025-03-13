class ExtraScriptFlags extends CInstanceDisplayOverrides {
   constructor() {
      super();
      this.criteria.push(new CInstanceDisplayOverrideCriteria("gSaveBlock1Ptr->flags[*]"));
      this.flag_names = null; // Map<int, String>
      this.trainers   = {
         count:      null,
         first_flag: null, // index
         last_flag:  null, // index
         names:      null, // Map<int, String>
      };
      
      this.#display_string_bound      = this.#display_string.bind(this);
      this.#make_editor_element_bound = this.#make_editor_element.bind(this);
   }
   
   // Expects flags.dat.
   finalize(/*ExtraDataFile*/ file) {
      if (!file) {
         return;
      }
      {
         let enumeration = file.found.enums.get("TRAINER_");
         if (enumeration) {
            this.trainers.names      = enumeration.members.by_value;
            this.trainers.count      = file.found.vars.get("TRAINERS_COUNT");
            this.trainers.first_flag = file.found.vars.get("TRAINER_FLAGS_START");
            this.trainers.last_flag  = file.found.vars.get("TRAINER_FLAGS_END");
         }
      }
      {
         let enumeration = file.found.enums.get("FLAG_");
         if (enumeration) {
            this.flag_names = enumeration.members.by_value;
         }
      }
      if (this.trainers.names && this.flag_names) {
         this.overrides.display_string      = this.#display_string_bound;
         this.overrides.make_editor_element = this.#make_editor_element_bound;
      }
   }
   
   get_name(n) {
      let text = this.flag_names.get(n);
      if (text) {
         return text;
      }
      {
         let count = this.trainers.count;
         let first = this.trainers.first_flag;
         let last  = this.trainers.last_flag;
         if (!first || !last)
            return null;
         if (n >= first && n <= last) {
            n -= first;
            let name = this.trainers.names.get(n);
            if (name)
               return "Trainer flag: " + name;
            if (count && n >= count) {
               return "Unused trainer flag 0x" + n.toString(16).toUpperCase();
            }
         }
      }
      return null;
   }
   
   #display_string_bound;
   #display_string(/*CInstance*/ inst) {
      let v = inst.value;
      if (v === null)
         return null;
      
      let bits_per = inst.decl.options.bitcount;
      let index    = inst.is_member_of.values.indexOf(inst);
      
      let offset = bits_per * index;
      let disp   = "";
      for(let i = 0; i < bits_per; ++i) {
         let bit = 1 << i;
         if (v & bit) {
            let text = this.get_name(offset + i);
            if (!text)
               text = (offset + i) + "";
            
            if (disp)
               disp += " | ";
            disp += `[style=value-text]${text}[/style]`;
         }
      }
      if (!disp)
         return "[style=value-text]0[/style]";
      return disp;
   }
   
   #make_editor_element_bound;
   #make_editor_element(/*CInstance*/ inst) {
      let bits_per = inst.decl.options.bitcount;
      let index    = inst.is_member_of.values.indexOf(inst);
      
      let offset = bits_per * index;
      let names  = new Map();
      for(let i = 0; i < bits_per; ++i) {
         let name = this.get_name(offset + i);
         if (name) {
            name = name.replaceAll("_", "_\u200B"); // allow word-wrap at underscores
            names.set(i, name);
         }
      }
      
      let node = document.createElement("bitset-input");
      node.bitcount = bits_per;
      node.labels   = names;
      return node;
   }
};