import { CInstanceDisplayOverrides, CInstanceDisplayOverrideCriteria } from "../c/c-instance-display-overrides.js";

export default class ExtraEasyChat extends CInstanceDisplayOverrides {
   constructor(/*ExtraDataCollection*/ collection) {
      super();
      this.extra_data_collection = collection;
      
      this.criteria.push(new CInstanceDisplayOverrideCriteria({ type: "EasyChatWordID" }));
      
      this.enumerations = {
         groups: null,
         words:  null,
      };
      
      this.word_names  = null; // Map<int, String>
      this.word_values = null; // Map<String, int>
      this.groups = {
         moves1:   null,
         moves2:   null,
         pokemon:  null,
         pokemon2: null,
      };
      
      this.word_index_bits = null;
      
      this.#display_string_bound      = this.#display_string.bind(this);
      this.#make_editor_element_bound = this.#make_editor_element.bind(this);
   }
   
   // Expects easy-chat.dat.
   finalize(/*Optional<ExtraDataFile>*/ easy_chat) {
      if (!easy_chat) {
         return;
      }
      this.word_index_bits = easy_chat.found.vars.get("EC_MASK_BITS") || null;
      {
         let enumeration = easy_chat.found.enums.get("EC_WORD_");
         if (enumeration) {
            this.enumerations.words = enumeration;
            this.word_names  = enumeration.members.by_value;
            this.word_values = enumeration.members.by_name;
         }
      }
      {
         let enumeration = easy_chat.found.enums.get("EC_GROUP_");
         if (enumeration) {
            this.enumerations.groups = enumeration;
            this.groups.pokemon  = enumeration.members.by_name.get("POKEMON");
            this.groups.pokemon2 = enumeration.members.by_name.get("POKEMON2");
            this.groups.moves1   = enumeration.members.by_name.get("MOVE_1");
            this.groups.moves2   = enumeration.members.by_name.get("MOVE_2");
         }
      }
      this.overrides.display_string      = this.#display_string_bound;
      this.overrides.make_editor_element = this.#make_editor_element_bound;
   }
   
   #split_word_value(v) {
      return {
         group: v >> this.word_index_bits,
         index: v & ((1 << this.word_index_bits) - 1)
      };
   }
   
   get_name(n) {
      let text = this.word_names.get(n);
      if (text) {
         return text;
      }
      if (!this.word_index_bits)
         return null;
      
      let {group, index} = this.#split_word_value(n);
      
      let group_name = null;
      let index_name = null;
      if (group === this.groups.moves1 || group === this.groups.moves2) {
         group_name = "EC_GROUP_MOVE_1";
         if (group === this.groups.moves2)
            group_name = "EC_GROUP_MOVE_2";
         
         let enumeration = this.extra_data_collection.enums.get("MOVE");
         if (enumeration) {
            index_name = enumeration.members.by_value.get(index);
         }
      } else if (group === this.groups.pokemon || group === this.groups.pokemon2) {
         group_name = "EC_GROUP_POKEMON";
         if (group === this.groups.moves2)
            group_name = "EC_GROUP_POKEMON_NATIONAL";
         
         let enumeration = this.extra_data_collection.enums.get("SPECIES");
         if (enumeration) {
            index_name = enumeration.members.by_value.get(index);
         }
      }
      if (group_name) {
         if (index_name)
            return `EC_WORD(${group_name}, ${index_name})`;
         return `EC_WORD(${group_name}, ${index})`;
      }
      return null;
   }
   
   #display_string_bound;
   #display_string(/*CInstance*/ inst) {
      let v = inst.value;
      if (v === null || +v !== v)
         return null;
      
      if (v === 0) {
         return "0";
      }
      
      let name = this.get_name(v);
      if (name) {
         return `[style=value-text]${name}[/style]`;
      }
      return null;
   }
   
   #make_editor_element_bound;
   #make_editor_element(/*CInstance*/ inst) {
      if (!this.enumerations.groups || !this.enumerations.words)
         return null;
      
      let node = document.createElement("select");
      
      let group_nodes = [];
      this.enumerations.words.members.by_name.forEach((function(v, k) {
         let {group, index} = this.#split_word_value(v);
         let gn = group_nodes[group];
         if (!group_nodes[group]) {
            gn = group_nodes[group] = document.createElement("optgroup");
            
            let name = this.enumerations.groups.members.by_value.get(group);
            gn.setAttribute("label", name || `#${group}`);
         }
         let wn = document.createElement("option");
         wn.setAttribute("value", v);
         wn.textContent = k;
         gn.append(wn);
      }).bind(this));
      
      function _paired_group_from_enum(e, name_a, name_b) {
         let gn1 = document.createElement("optgroup");
         gn1.setAttribute("label", name_a);
         
         if (e) {
            let sorted = [...e.members.by_name.entries()].sort(function(a, b) {
               return a[0] < b[0];
            });
            for(let pair of sorted) {
               let wn = document.createElement("option");
               wn.setAttribute("value", pair[1]);
               wn.textContent = pair[0];
               gn1.append(wn);
            }
         }
         
         let gn2 = gn1.cloneNode(true);
         gn2.setAttribute("label", name_b);
         
         return [gn1, gn2];
      }
      //
      {
         let pair = _paired_group_from_enum(
            this.extra_data_collection.enums.get("MOVE"),
            "MOVE_1",
            "MOVE_2"
         );
         group_nodes[this.groups.moves1] = pair[0];
         group_nodes[this.groups.moves2] = pair[1];
      }
      {
         let pair = _paired_group_from_enum(
            this.extra_data_collection.enums.get("SPECIES"),
            "POKEMON",
            "POKEMON_NATIONAL"
         );
         group_nodes[this.groups.pokemon]  = pair[0];
         group_nodes[this.groups.pokemon2] = pair[1];
      }
      
      for(let i = 0; i < group_nodes.length; ++i) {
         let gn = group_nodes[i];
         if (gn)
            node.append(gn);
      }
      return node;
   }
};