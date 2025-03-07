
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
   make_enum( // Battle Tower level mode
      [
         new CInstanceDisplayOverrideCriteria({
            type: "Apprentice",
            path: "lvlMode",
         }),
         new CInstanceDisplayOverrideCriteria({
            type: "BattleFrontier",
            path: "lvlMode",
         }),
         new CInstanceDisplayOverrideCriteria({
            type: "BattleFrontier",
            path: "towerLvlMode",
         }),
         new CInstanceDisplayOverrideCriteria({
            type: "BattleFrontier",
            path: "domeLvlMode",
         }),
         new CInstanceDisplayOverrideCriteria({
            type: "EmeraldBattleTowerRecord",
            path: "lvlMode",
         }),
         new CInstanceDisplayOverrideCriteria({
            type: "PlayersApprentice",
            path: "lvlMode",
         }),
      ],
      [  // include/constants/global.h
         "FRONTIER_LVL_50",
         "FRONTIER_LVL_OPEN",
         "FRONTIER_LVL_TENT",
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