import { CInstanceDisplayOverrides, CInstanceDisplayOverrideCriteria } from "../c-instance-display-overrides.js";

export default class ExtraScriptVars extends CInstanceDisplayOverrides {
   constructor() {
      super();
      this.criteria.push(new CInstanceDisplayOverrideCriteria("gSaveBlock1Ptr->vars[*]"));
      this.names = null; // Map<int runtime_var_index, String>
      this.start = null; // runtime index of the first var (0x4000 in vanilla)
      
      this.#display_index_bound = this.#display_index.bind(this);
   }
   
   // Expects vars.dat.
   finalize(/*ExtraDataFile*/ file) {
      if (!file) {
         return;
      }
      let enumeration = file.found.enums.get("VAR_");
      let start_index = file.found.vars.get("VARS_START");
      if (enumeration && (start_index || start_index === 0)) {
         this.names = enumeration.members.by_value;
         this.start = start_index;
         this.overrides.display_index = this.#display_index_bound;
      }
   }
   
   get_name(n) {
      return this.names.get(n + this.start);
   }
   
   #display_index_bound;
   #display_index(/*CInstance*/ inst) {
      let index = inst.is_member_of.values.indexOf(inst);
      let name  = this.get_name(index);
      if (name)
         return "0x" + (index + this.start).toString(16).toUpperCase() + ": " + name;
      return null;
   }
};