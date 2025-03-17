// Extra data pertaining to a given savedata serialization version.
class ExtraDataCollection {
   #enum_overrides;
   #ready_promise;
   
   constructor(ready_promise) {
      this.constants    = new Map();
      this.enums        = new Map();
      this.game_stats   = new ExtraGameStats();
      this.map_data     = new ExtraCoalescedMapsData();
      this.script_flags = new ExtraScriptFlags();
      this.script_vars  = new ExtraScriptVars();
      
      this.#enum_overrides = new Map();
      this.#ready_promise  = ready_promise;
   }
   
   get readyPromise() {
      return this.#ready_promise;
   }
   
   static #enum_display_criteria = {
      "CONTEST_RANK": [
         new CInstanceDisplayOverrideCriteria({
            type: "ContestWinner",
            path: "contestRank",
         }),
      ],
   };
   
   finalize(/*Map<String filename, ExtraDataFile>*/ files) {
      this.game_stats.finalize(files.get("game-stats.dat"));
      this.script_flags.finalize(files.get("flags-#.dat"), files.get("trainers.dat"));
      this.script_vars.finalize(files.get("vars.dat"));
      {
         let desired_constants = {
            "misc.dat": [
               // Sentinel values for Pokemon species gender ratios, indicating 
               // that all members of the species have a fixed gender.
               "MON_MALE",
               "MON_FEMALE",
               "MON_GENDERLESS",
               
               "SHINY_ODDS",
            ],
         };
         for(let filename in desired_constants) {
            let file = files.get(filename);
            if (!file)
               continue;
            for(let name of desired_constants[filename]) {
               let data = file.found.vars.get(name);
               if (data || data === 0)
                  this.constants.set(name, data);
            }
         }
      }
      {
         let desired_enums = {
            "misc.dat": [
               "CONTEST_CATEGORY",
               "CONTEST_RANK",
               "GROWTH",
            ],
            "items.dat": [
               "ITEM",
            ],
            "moves.dat": [
               "MOVE",
            ],
            "natures.dat": [
               "NATURE",
            ],
            "species.dat": [
               "SPECIES",
            ],
         };
         for(let filename in desired_enums) {
            let file = files.get(filename);
            if (!file)
               continue;
            for(let name of desired_enums[filename]) {
               let data = file.found.enums.get(name + '_');
               if (data)
                  this.enums.set(name, data);
            }
         }
      }
      //
      // Create display overrides for basic enums.
      //
      {
         let list = ExtraDataCollection.#enum_display_criteria;
         for(let name in list) {
            let data = this.enums.get(name);
            if (!data)
               continue;
            let disp = data.make_display_overrides(list[name]);
            this.#enum_overrides.set(name, disp);
         }
      }
      {
         let file = files.get("maps.dat");
         if (file && file.found.map_data) {
            this.map_data.finalize(file);
         }
      }
   }
   
   apply(/*SaveFormat*/ format) {
      format.display_overrides = format.display_overrides.concat(
         [
            this.game_stats,
            this.script_flags,
            this.script_vars,
         ],
         Object.values(this.map_data.overrides),
         Array.from(this.#enum_overrides.values())
      );
      
      let _set_enum_type = (function(typename, enum_name) {
         let ed = this.enums.get(enum_name);
         if (ed)
            format.enums.set(typename, ed.members.by_name);
      }).bind(this);
      _set_enum_type("ContestCategory",  "CONTEST_CATEGORY");
      _set_enum_type("ItemIDGlobal",     "ITEM");
      _set_enum_type("MoveID",           "MOVE");
      _set_enum_type("PokemonSpeciesID", "SPECIES");
   }
};