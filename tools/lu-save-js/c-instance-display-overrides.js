// Control how certain CInstances are displayed. Specify criteria 
// that a CInstance must match, and then overrides for their display.
class CInstanceDisplayOverrides {
   constructor(o) {
      this.criteria = {
         decl: o?.criteria?.decl || null, // Optional<CDeclInstance>
         path: o?.criteria?.path || null, // Optional<String>
         type: o?.criteria?.type || null, // Optional<CTypeInstance>
      };
      this.overrides = {
         // A function which returns a BBCode string. The following tags are 
         // supported: b; i; u; color; style; where `style` takes a single 
         // argument: `name-text` or `value-text`. The function will receive 
         // the CInstance being displayed as its sole argument.
         display_string: o?.overrides?.display_string || null,
         
         // If set, a function which returns a new HTML element. The element 
         // will be used by CValueEditorElement and must allow [gs]etting its 
         // `value` property. The function will receive the CInstance being 
         // edited as its sole argument.
         make_editor_element: o?.overrides?.make_editor_element || null,
      };
      if (this.criteria.path && !(this.criteria.path instanceof CValuePath)) {
         this.criteria.path = new CValuePath(this.criteria.path);
      }
   }
   
   matches(/*CInstance*/ inst) {
      let desired_decl = this.criteria.decl;
      let desired_path = this.criteria.path;
      let desired_type = this.criteria.type;
      if (!desired_path) {
         if (desired_decl && inst.decl !== desired_decl)
            return false;
         if (desired_type) {
            if (inst instanceof CTypeInstance) {
               return inst.type === desired_type;
            } else {
               return inst.decl.c_types.serialized.definition === desired_type;
            }
         }
         return false;
      }
      return desired_path.matches(
         inst,
         function(base) {
            if (desired_decl && base.decl !== desired_decl)
               return false;
            if (desired_type) {
               if (base instanceof CTypeInstance) {
                  return base.type === desired_type;
               } else {
                  return base.decl.c_types.serialized.definition === desired_type;
               }
            }
            return true;
         }
      );
   }
};