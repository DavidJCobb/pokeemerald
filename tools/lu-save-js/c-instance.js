class CInstance {
   constructor(decl, type) {
      refuse_abstract_instantiation(CInstance);
      this.decl = decl;         // CDeclDefinition
      this.type = type || null; // Optional<CTypeDefinition>
      
      this.is_member_of = null; // Optional<CInstance>
   }
   
   // Identifier of the VAR_DECL or FIELD_DECL this CInstance represents.
   get identifier() {
      return this.decl.name || null;
   }
   
   get save_format() {
      return this.decl.save_format;
   }
   
   build_path_string() {
      let parts = [this];
      for(let inst = this.is_member_of; inst; inst = inst.is_member_of)
         parts.push(inst);
      parts = parts.reverse();
      
      let path = "";
      for(let i = 0; i < parts.length; ++i) {
         const inst = parts[i];
         const prev = parts[i - 1];
         if (inst instanceof SaveSlot)
            continue;
         
         const is_array_element = prev instanceof CArrayInstance;
         if (is_array_element) {
            let index = prev.values.indexOf(inst);
            console.assert(index >= 0);
            path += `[${index}]`;
            continue;
         }
         if (prev && !(prev instanceof SaveSlot)) {
            path += '.';
         }
         let segm = inst.identifier || "__unnamed";
         {
            let deref = inst.decl?.dereference_count;
            if (deref && deref > 0) {
               segm = `(${"".padStart(deref, '*')}${segm})`;
            }
         }
         path += segm;
      }
      return path;
   }
};