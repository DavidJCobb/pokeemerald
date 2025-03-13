class CInstanceDisplayOverrideCriteria {
   constructor(o) {
      this.decl = o?.decl || null; // Optional<CDeclInstance>
      this.path = o?.path || null; // Optional<String>
      this.type = o?.type || null; // Optional<Variant<CTypeInstance, String typename>>
      if (o+"" === o) {
         this.path = o;
      }
      if (this.path && !(this.path instanceof CValuePath)) {
         this.path = new CValuePath(this.path);
      }
   }
   
   #instance_type_matches(/*CInstance*/ inst) {
      if (!this.type)
         return true;
      let inst_type;
      if (inst instanceof CTypeInstance) {
         inst_type = inst.type;
      } else {
         inst_type = inst.decl.c_types.serialized_type;
      }
      if (this.type+"" === this.type) {
         return inst_type && (inst_type.tag == this.type || inst_type.symbol == this.type);
      }
      return inst_type === this.type;
   }
   
   matches(/*CInstance*/ inst) {
      let desired_decl = this.decl;
      let desired_path = this.path;
      let desired_type = this.type;
      if (!desired_path) {
         if (desired_decl && inst.decl !== desired_decl)
            return false;
         if (desired_type)
            return this.#instance_type_matches(inst);
         return false;
      }
      return desired_path.matches(
         inst,
         (function(base) {
            if (desired_decl && base.decl !== desired_decl)
               return false;
            return this.#instance_type_matches(base);
         }).bind(this)
      );
   }
};

// Control how certain CInstances are displayed. Specify criteria 
// that a CInstance must match, and then overrides for their display.
class CInstanceDisplayOverrides {
   constructor(o) {
      this.criteria  = o?.criteria || []; // Array<CInstanceDisplayOverrideCriteria>
      this.overrides = {
         // A function which returns a BBCode string. The following tags are 
         // supported: b; i; u; color; style; where `style` takes a single 
         // argument: `name-text` or `value-text`. The function will receive 
         // the CInstance being displayed as its sole argument.
         //
         // This function influences how a CInstance's value is rendered.
         display_string: o?.overrides?.display_string || null,
         
         // A function which returns a plaintext string.
         //
         // When a CInstance is an array element, this function overrides the 
         // display of the array index.
         display_index: o?.overrides?.display_index || null,
         
         // If set, a function which returns a new HTML element. The element 
         // will be used by CValueEditorElement and must allow [gs]etting its 
         // `value` property. The function will receive the CInstance being 
         // edited as its sole argument.
         make_editor_element: o?.overrides?.make_editor_element || null,
      };
      
      if (!Array.isArray(this.criteria)) {
         if (this.criteria instanceof CInstanceDisplayOverrideCriteria) {
            this.criteria = [this.criteria];
         } else {
            this.criteria = [ new CInstanceDisplayOverrideCriteria(this.criteria) ];
         }
      }
   }
   
   matches(/*CInstance*/ inst) {
      for(let crit of this.criteria)
         if (crit.matches(inst))
            return true;
      return false;
   }
};