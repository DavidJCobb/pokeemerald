class ExtraCoalescedMapsData extends CInstanceDisplayOverrides {
   constructor() {
      super();
      this.map_groups        = null;
      this.map_section_names = null; // Optional<Map<int, String>>
      this.overrides = {
         map_section: new ExtraCoalescedMapsData.MapSectionDisplayOverrides(this),
         map_group:   new ExtraCoalescedMapsData.MapGroupDisplayOverrides(this),
         map_num:     new ExtraCoalescedMapsData.MapNumDisplayOverrides(this),
      };
   }
   
   // Expects maps.dat.
   finalize(/*Optional<ExtraDataFile>*/ file) {
      if (!file) {
         return;
      }
      if (file.found.map_data) {
         this.map_groups = file.found.map_data.map_groups;
      }
      {
         let enumeration = file.found.enums.get("MAPSEC_");
         if (enumeration)
            this.map_section_names = new Map(enumeration.members.by_value);
      }
      {
         let enumeration = file.found.enums.get("METLOC_");
         if (enumeration)
            for(const [k, v] of enumeration.members.by_name)
               this.map_section_names.set(v, k);
      }
   }
   
   #map_section_disp_override;
   #map_group_disp_override;
   #map_disp_override;
   
   map_section_name(n) {
      if (!this.map_section_names)
         return null;
      return this.map_section_names.get(n) || null;
   }
   map_group_name(n) {
      if (!this.map_groups)
         return null;
      return this.map_groups[n]?.name || null;
   }
   
   static MapSectionDisplayOverrides = class MapSectionDisplayOverrides extends CInstanceDisplayOverrides {
      #owner;
      
      constructor(/*ExtraCoalescedMapsData*/ owner) {
         super();
         this.#owner = owner;
         this.overrides = {
            display_string:      this.#display_string.bind(this),
            make_editor_element: this.#make_editor_element.bind(this),
         };
         this.criteria.push(new CInstanceDisplayOverrideCriteria({
            type: "MapSectionID"
         }));
         this.criteria.push(new CInstanceDisplayOverrideCriteria({
            type: "MetLocation"
         }));
      }
      
      #display_string(/*CInstance*/ inst) {
         let v = inst.value;
         if (v === null)
            return null;
         let name = this.#owner.map_section_name(v);
         if (name)
            return `[style=value-text]${name}[/style]`;
         return null;
      }
      #make_editor_element(/*CInstance*/ inst) {
         if (!this.#owner.map_section_names)
            return null;
         let node = document.createElement("enum-input");
         let data = new Map();
         for(const [k, v] of this.#owner.map_section_names)
            data.set(v, k);
         node.valueNames = data;
         return node;
      }
   };
   
   static MapGroupDisplayOverrides = class MapGroupDisplayOverrides extends CInstanceDisplayOverrides {
      #owner;
      
      constructor(/*ExtraCoalescedMapsData*/ owner) {
         super();
         this.#owner = owner;
         this.overrides = {
            display_string:      this.#display_string.bind(this),
            make_editor_element: this.#make_editor_element.bind(this),
         };
         this.criteria.push(new CInstanceDisplayOverrideCriteria({
            type: "MapGroupIndex"
         }));
         this.criteria.push(new CInstanceDisplayOverrideCriteria({
            type: "MapGroupIndexOptional"
         }));
      }
      
      #display_string(/*CInstance*/ inst) {
         let v = inst.value;
         if (v === null)
            return null;
         let name = this.#owner.map_group_name(v);
         if (name)
            return `[style=value-text]${name}[/style]`;
         return null;
      }
      #make_editor_element(/*CInstance*/ inst) {
         if (!this.#owner.map_groups)
            return null;
         let node = document.createElement("enum-input");
         let data = new Map();
         for(let i = 0; i < this.#owner.map_groups.length; ++i) {
            data.set(this.#owner.map_groups[i].name, i);
         }
         node.valueNames = data;
         return node;
      }
   };
   
   static MapNumDisplayOverrides = class MapNumDisplayOverrides extends CInstanceDisplayOverrides {
      #owner;
      
      constructor(/*ExtraCoalescedMapsData*/ owner) {
         super();
         this.#owner = owner;
         this.overrides = {
            display_string:      this.#display_string.bind(this),
            make_editor_element: this.#make_editor_element.bind(this),
         };
         this.criteria.push(new CInstanceDisplayOverrideCriteria({
            type: "MapIndexInGroup",
         }));
         this.criteria.push(new CInstanceDisplayOverrideCriteria({
            type: "MapIndexInGroupOptional",
         }));
      }
      
      #get_map_group(/*CInstance*/ map_num_inst) {
         if (!this.#owner.map_groups)
            return null;
         let inside = map_num_inst.is_member_of;
         if (!inside)
            return null;
         let v;
         {
            let desired = null;
            for(let annotation of map_num_inst.decl.annotations) {
               let match = annotation.match(/^map-group\(([^\)]+)\)$/);
               if (match) {
                  desired = match[1];
                  break;
               }
            }
            let memb;
            if (desired) {
               memb = inside.members[desired];
            } else {
               let multiple = false;
               for(let name in inside.members) {
                  let here = inside.members[name];
                  let tn   = here.decl.serialized_type?.symbol;
                  if (tn == "MapGroupIndex" || tn == "MapGroupIndexOptional") {
                     if (memb) {
                        multiple = true;
                        break;
                     } else {
                        memb = here;
                     }
                  }
               }
               if (multiple)
                  memb = null;
            }
            if (memb) {
               v = memb.value;
               if (v === null)
                  return null;
            } else {
               return null;
            }
         }
         return this.#owner.map_groups[v];
      }
      
      #display_string(/*CInstance*/ inst) {
         let n = inst.value;
         if (n === null)
            return null;
         let g = this.#get_map_group(inst);
         if (!g)
            return null;
         let name = g.maps[n];
         if (name)
            return `[style=value-text]${name}[/style]`;
         return null;
      }
      #make_editor_element(/*CInstance*/ inst) {
         let g = this.#get_map_group(inst);
         if (!g)
            return null;
         let node = document.createElement("enum-input");
         node.valueNames = g.maps;
         return node;
      }
   };
};