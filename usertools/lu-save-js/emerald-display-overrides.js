
const EMERALD_DISPLAY_OVERRIDES = (function() {
   let overrides = [];
   
   let make_enum;
   {
      function display_string(enumeration, /*CInstance*/ inst) {
         let v = inst.value;
         if (v === null)
            return null;
         if (v >= 0 && v < enumeration.length)
            return "[style=value-text]" + enumeration[v] + "[/style]";
         return null;
      }
      function make_editor_element(enumeration, /*CInstance*/ inst) {
         let node = document.createElement("enum-input");
         node.valueNames = enumeration;
         return node;
      }
      
      make_enum = function(criteria, enumeration) {
         overrides.push(new CInstanceDisplayOverrides({
            criteria:  criteria,
            overrides: {
               display_string:      display_string.bind(null, enumeration),
               make_editor_element: make_editor_element.bind(null, enumeration),
            }
         }));
      };
   }
   
   make_enum( // Battle Tower facility class
      [
         new CInstanceDisplayOverrideCriteria({
            type: "BattleTowerEReaderTrainer",
            path: "facilityClass",
         }),
         new CInstanceDisplayOverrideCriteria({
            type: "EmeraldBattleTowerRecord",
            path: "facilityClass",
         }),
      ],
      [  // include/constants/battle_frontier.h
         "FRONTIER_FACILITY_TOWER",
         "FRONTIER_FACILITY_DOME",
         "FRONTIER_FACILITY_PALACE",
         "FRONTIER_FACILITY_ARENA",
         "FRONTIER_FACILITY_FACTORY",
         "FRONTIER_FACILITY_PIKE",
         "FRONTIER_FACILITY_PYRAMID",
         "FACILITY_LINK_CONTEST",
         "FACILITY_UNION_ROOM",
         "FACILITY_MULTI_OR_EREADER",
      ]
   );
   make_enum( // PokeNews kind
      [
         new CInstanceDisplayOverrideCriteria({
            type: "PokeNews",
            path: "kind",
         }),
      ],
      [  // include/constants/tv.h
         "POKENEWS_NONE",
         "POKENEWS_SLATEPORT",
         "POKENEWS_GAME_CORNER",
         "POKENEWS_LILYCOVE",
         "POKENEWS_BLENDMASTER",
      ]
   );
   make_enum( // PokeNews state
      [
         new CInstanceDisplayOverrideCriteria({
            type: "PokeNews",
            path: "state",
         }),
      ],
      [  // include/constants/tv.h
         "POKENEWS_STATE_INACTIVE",
         "POKENEWS_STATE_UPCOMING",
         "POKENEWS_STATE_ACTIVE",
      ]
   );
   overrides.push(new CInstanceDisplayOverrides({ // PokemonSubstruct1
      criteria: [
         new CInstanceDisplayOverrideCriteria({ type: "PokemonSubstruct1" }),
      ],
      overrides: {
         display_string: function(/*CInstance*/ inst) {
            if (!inst.members)
               return null;
            let moves = inst.members.moves;
            let pp    = inst.members.pp;
            if (!moves || !pp || !(moves instanceof CArrayInstance && pp instanceof CArrayInstance))
               return null;
            let size = Math.min(moves.values.length, pp.values.length);
            
            let s = "[raw][[/raw] ";
            for(let i = 0; i < size; ++i) {
               let mv = moves.values[i];
               let pv = pp.values[i];
               if (!mv || !pv || mv.value === null || !pv.value === null)
                  return null;
               if (i) {
                  s += ", ";
               }
               
               let move_name = mv.value;
               {
                  let format   = mv.save_format;
                  let enum_def = format?.enums.get(mv.decl.c_types.serialized.name);
                  if (enum_def) {
                     for(const [name, value] of enum_def) {
                        if (value == move_name) {
                           move_name = name;
                           break;
                        }
                     }
                  }
               }
               s += `[style=value-text]${move_name}[/style]`;
               s += " x";
               s += `[style=value-text]${pv.value}[/style]`;
            }
            s += " ]";
            return s;
         },
      }
   }));
   overrides.push(new CInstanceDisplayOverrides({ // Secret Base decoration position
      criteria: [
         new CInstanceDisplayOverrideCriteria({
            type: "SaveBlock1",
            path: "playerRoomDecorationPositions[*]",
         }),
         new CInstanceDisplayOverrideCriteria({
            type: "SecretBase",
            path: "decorationPositions[*]",
         }),
      ],
      overrides: {
         display_string: function(/*CInstance*/ inst) {
            let v = inst.value;
            if (v === null)
               return null;
            return `([style=value-text]${(v >> 4) & 0x0F}[/style], [style=value-text]${v & 0x0F}[/style])`;
         },
      }
   }));
   
   return overrides;
})();