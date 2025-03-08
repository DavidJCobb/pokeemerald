class ExtraGameStats extends CInstanceDisplayOverrides {
   constructor() {
      super();
      this.criteria.push(new CInstanceDisplayOverrideCriteria("gSaveBlock1Ptr->gameStats[*]"));
      this.names = null; // Map<int runtime_var_index, String>
      this.count = null;
      this.used  = null; // number of game stats that are actually used
      
      this.#display_index_bound = this.#display_index.bind(this);
   }
   
   // Expects vars.dat.
   finalize(/*ExtraDataFile*/ file) {
      if (!file) {
         return;
      }
      let enumeration = file.found.enums.get("GAME_STAT_");
      this.count = file.found.vars.get("NUM_GAME_STATS");
      this.used  = file.found.vars.get("NUM_USED_GAME_STATS");
      if (enumeration && this.count) {
         this.names = enumeration.members.by_value;
         this.overrides.display_index = this.#display_index_bound;
      }
   }
   
   get_name(n) {
      let name = this.names.get(n);
      if (name)
         return name;
      if (n >= this.used && n < this.count)
         return "Unused game stat";
   }
   
   #display_index_bound;
   #display_index(/*CInstance*/ inst) {
      let index = inst.is_member_of.values.indexOf(inst);
      let name  = this.get_name(index);
      if (name)
         return name + " (" + index + ")";
      return null;
   }
};