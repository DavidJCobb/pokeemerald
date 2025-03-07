
class ExtraDataCollection {
   #promise;
   #overrides;
   
   constructor(promise) {
      this.#promise   = promise;
      this.#overrides = [];
      this.enums = {
         CONTEST_CATEGORY: null,
         CONTEST_RANK:     null,
         FLAG:      null,
         GAME_STAT: null,
         GROWTH:    null,
         ITEM:      null,
         MOVE:      null,
         NATURE:    null,
         SPECIES:   null,
         TRAINER:   null,
         VAR:       null,
      };
      this.vars = new Map();
   }
   
   get promise() { return this.#promise; }
   
   #enum_display_string(enumeration, /*CInstance*/ inst) {
      let v = inst.value;
      if (v === null)
         return null;
      if (v >= 0 && v < enumeration.length)
         return "[style=value-text]" + enumeration[v] + "[/style]";
      return null;
   }
   #enum_make_editor_element(enumeration, /*CInstance*/ inst) {
      let node = document.createElement("enum-input");
      node.valueNames = enumeration;
      return node;
   }
   #make_enum_display_overrides(criteria, enumeration) {
      this.#overrides.push(new CInstanceDisplayOverrides({
         criteria:  criteria,
         overrides: {
            display_string:      this.#enum_display_string.bind(null, enumeration),
            make_editor_element: this.#enum_make_editor_element.bind(null, enumeration),
         }
      }));
   }
   
   finalize() {
      if (this.enums.CONTEST_CONTEST) {
         this.#make_enum_display_overrides(
            [
               new CInstanceDisplayOverrideCriteria({
                  type: "ContestWinner",
                  path: "contestCategory",
               }),
               new CInstanceDisplayOverrideCriteria({
                  type: "LilycoveLadyContest",
                  path: "category",
               }),
            ],
            this.enums.CONTEST_CONTEST
         );
      }
      if (this.enums.CONTEST_RANK) {
         this.#make_enum_display_overrides(
            [
               new CInstanceDisplayOverrideCriteria({
                  type: "ContestWinner",
                  path: "contestRank",
               }),
            ],
            this.enums.CONTEST_RANK
         );
      }
      if (this.enums.FLAG) {
         const _values_to_names = new Map();
         for (const [name, value] of this.enums.FLAG)
            _values_to_names.set(value, name);
         
         let _get_flag_name;
         {
            let trainer_flags_range;
            if (this.enums.TRAINER) {
               trainer_flags_range = {
                  first: this.vars.get("TRAINER_FLAGS_START"),
                  last:  this.vars.get("TRAINER_FLAGS_END"),
               };
            }
            let trainer_flags_count = this.vars.get("TRAINERS_COUNT");
            if (trainer_flags_range && trainer_flags_range.first && trainer_flags_range.last) {
               const _trainer_names = new Map();
               for(const [k, v] of this.enums.TRAINER)
                  _trainer_names.set(v, k);
               _get_flag_name = function(n) {
                  let text = _values_to_names.get(n);
                  if (text)
                     return text;
                  if (n >= trainer_flags_range.first && n <= trainer_flags_range.last) {
                     let name = _trainer_names.get(n - trainer_flags_range.first);
                     if (name)
                        return "Trainer flag: " + name;
                     if (trainer_flags_count && n - trainer_flags_range.first >= trainer_flags_count) {
                        return "Unused trainer flag 0x" + (n - trainer_flags_range.first).toString(16).toUpperCase();
                     }
                  }
               };
            } else {
               _get_flag_name = function(n) {
                  return _values_to_names.get(n);
               };
            }
         }
         
         this.#overrides.push(new CInstanceDisplayOverrides({
            criteria: [new CInstanceDisplayOverrideCriteria({
               path: "gSaveBlock1Ptr->flags[*]",
            })],
            overrides: {
               display_string: function(/*CInstance*/ inst) {
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
                        let text = _get_flag_name(offset + i);
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
               },
               
               make_editor_element: function(/*CInstance*/ inst) {
                  let bits_per = inst.decl.options.bitcount;
                  let index    = inst.is_member_of.values.indexOf(inst);
                  
                  let offset = bits_per * index;
                  let names  = new Map();
                  for(let i = 0; i < bits_per; ++i) {
                     let name = _get_flag_name(offset + i);
                     if (name) {
                        name = name.replaceAll("_", "_\u200B"); // allow word-wrap at underscores
                        names.set(i, name);
                     }
                  }
                  
                  let node = document.createElement("bitset-input");
                  node.bitcount = 8;
                  node.labels   = names;
                  return node;
               },
            },
         }));
      }
      if (this.enums.VAR && this.vars.has("VARS_START")) {
         const _start_value     = this.vars.get("VARS_START");
         const _values_to_names = new Map();
         for (const [name, value] of this.enums.VAR)
            _values_to_names.set(value - _start_value, name);
         
         let _get_name = function(n) {
            return _values_to_names.get(n);
         };
         this.#overrides.push(new CInstanceDisplayOverrides({
            criteria: [new CInstanceDisplayOverrideCriteria({
               path: "gSaveBlock1Ptr->vars[*]",
            })],
            overrides: {
               display_index: function(/*CInstance*/ inst) {
                  let index = inst.is_member_of.values.indexOf(inst);
                  let name  = _get_name(index);
                  if (name)
                     return "0x" + (index + _start_value).toString(16).toUpperCase() + ": " + name;
                  return null;
               },
            },
         }));
      }
      if (this.enums.GAME_STAT) {
         const _values_to_names = new Map();
         for (const [name, value] of this.enums.GAME_STAT)
            _values_to_names.set(value, name);
         
         let _get_name = function(n) {
            return _values_to_names.get(n);
         };
         {
            let used = this.vars.get("NUM_USED_GAME_STATS");
            let max  = this.vars.get("NUM_GAME_STATS");
            if (used && max) {
               _get_name = function(n) {
                  let name = _values_to_names.get(n);
                  if (name)
                     return name;
                  if (n >= used && n < max)
                     return "Unused game stat";
               };
            }
         }
         this.#overrides.push(new CInstanceDisplayOverrides({
            criteria: [new CInstanceDisplayOverrideCriteria({
               path: "gSaveBlock1Ptr->gameStats[*]",
            })],
            overrides: {
               display_index: function(/*CInstance*/ inst) {
                  let index = inst.is_member_of.values.indexOf(inst);
                  let name  = _get_name(index);
                  if (name)
                     return name + " (" + index + ")";
                  return null;
               },
            },
         }));
      }
   }
   
   apply(/*SaveFormat*/ format) {
      format.display_overrides = format.display_overrides.concat(this.#overrides);
      format.enums.set("ItemIDGlobal",     this.enums.ITEM);
      format.enums.set("MoveID",           this.enums.MOVE);
      format.enums.set("PokemonSpeciesID", this.enums.SPECIES);
   }
};

let ExtraDataIndexManager = new (class ExtraDataIndexManager {
   constructor() {
      this.collections_by_serialization_version = new Map(); // Map<int, ExtraDataCollection>
   }
   async load_version(v) {
      let coll = this.collections_by_serialization_version.get(v);
      if (coll)
         return coll;
      
      let pi = Promise.withResolvers();
      try {
         coll = new ExtraDataCollection(pi.promise);
         this.collections_by_serialization_version.set(v, coll);
         
         let base_path = `./formats/${v}/`;
         
         let promises = {
            flags:   fetch(`${base_path}flags.dat`),
            game_stats: fetch(`${base_path}game-stats.dat`),
            items:   fetch(`${base_path}items.dat`),
            misc:    fetch(`${base_path}misc.dat`),
            moves:   fetch(`${base_path}moves.dat`),
            nature:  fetch(`${base_path}natures.dat`),
            species: fetch(`${base_path}species.dat`),
            vars:    fetch(`${base_path}vars.dat`),
         };
         try {
            await Promise.all(Object.values(promises));
         } catch (e) {
            // network error or we are running on file:///
            return coll;
         }
         
         let files = {};
         for(let key in promises) {
            let blob = await (await promises[key]).arrayBuffer();
            files[key] = new ExtraDataFile(blob);
         }
         
         coll.enums.CONTEST_CATEGORY = files.misc.found.enums.get("CONTEST_CATEGORY_") || null;
         coll.enums.CONTEST_RANK     = files.misc.found.enums.get("CONTEST_RANK_")     || null;
         coll.enums.FLAG    = files.flags.found.enums.get("FLAG_")      || null;
         coll.enums.GAME_STAT = files.game_stats.found.enums.get("GAME_STAT_") || null;
         coll.enums.GROWTH  = files.misc.found.enums.get("GROWTH_")     || null;
         coll.enums.ITEM    = files.items.found.enums.get("ITEM_")      || null;
         coll.enums.MOVE    = files.moves.found.enums.get("MOVE_")      || null;
         coll.enums.NATURE  = files.nature.found.enums.get("NATURE_")   || null;
         coll.enums.SPECIES = files.species.found.enums.get("SPECIES_") || null;
         coll.enums.TRAINER = files.flags.found.enums.get("TRAINER_")   || null;
         coll.enums.VAR     = files.vars.found.enums.get("VAR_")   || null;
         for(let k of [
            "TRAINER_FLAGS_START",
            "TRAINER_FLAGS_END",
            "TRAINERS_COUNT",
         ]) {
            let v = files.flags.found.vars.get(k);
            if (v || v === 0) {
               coll.vars.set(k, v);
            }
         }
         for(let k of [
            "NUM_GAME_STATS",
            "NUM_USED_GAME_STATS",
         ]) {
            let v = files.game_stats.found.vars.get(k);
            if (v || v === 0) {
               coll.vars.set(k, v);
            }
         }
         for(let k of [
            "VARS_START",
         ]) {
            let v = files.vars.found.vars.get(k);
            if (v || v === 0) {
               coll.vars.set(k, v);
            }
         }
         coll.finalize();
         
         pi.resolve(coll);
      } catch (e) {
         pi.reject(e);
         throw e;
      }
      return coll;
   }
})();