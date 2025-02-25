class CUnionInstance extends CTypeInstance {
   constructor(decl) {
      if (decl) {
         assert_type(decl instanceof CDeclDefinition);
         assert_logic(decl.serialized_type instanceof CUnionDefinition);
      }
      super(decl);
      this.value        = null;
      this.external_tag = null; // Optional<CValueInstance>
   }
   
   copy_contents_of(/*const CUnionInstance*/ other) {
      if (other.value === null) {
         this.value = null;
         return;
      }
      if (this.value === null) {
         if (this.external_tag) {
            let v = this.external_tag.value;
            if (v === null)
               throw new Error("cannot emplace this union (external tag not set)");
            let memb = this.type.members_by_tag_value[+tv];
            if (!memb)
               throw new Error("cannot emplace this union (external tag matches no member)");
            this.emplace(memb);
         } else {
            let memb = this.type.member_by_name(other.value.decl.name);
            if (!memb)
               throw new Error("cannot emplace this union (internally tagged; no member has the same name as the source's active member)");
            this.emplace(memb);
         }
      }
      this.value.copy_contents_of(other.value);
   }
   
   // Offered for use by user-defined translators.
   //
   // If the union is externally tagged:
   //
   //  - If the argument is non-empty, then throw.
   //
   //  - Ensure that the union has the appropriate active memver given its 
   //    tag value.
   //
   // If the union is internally tagged:
   //
   //  - If the argument is empty, then throw.
   //
   //  - Ensure that the requested member is the union's active member.
   //
   emplace(/*Optional<Variant<String, CDeclDefinition>>*/ member) {
      if (this.external_tag) {
         if (member)
            throw new Error("This union is externally tagged. You cannot request a specific active member; instead, change the tag value and then call this function without an argument.");
         
         let v = this.external_tag.value;
         if (v === null)
            throw new Error("This union is externally tagged. The external tag has no value.");
         v = +v;
         
         member = this.type.members_by_tag_value[v];
      } else {
         if (!member)
            throw new Error("This union is internally tagged. You must request a specific active member.");
      }
      this._emplace_for_load(member);
   }
   
   //
   // Internal use:
   //
   
   /*CInstance*/ #emplace(/*Variant<String, CDeclDefinition>*/ member) {
      if (member+"" === member)
         member = this.type.member_by_name(member);
      console.assert(member instanceof CDeclDefinition);
      if (!this.type.members.includes(member)) {
         throw new Error("Failed to emplace a CUnionInstance's value: the specified member CDeclDefinition does not belong to this union's type.");
      }
      
      this.value = member.make_instance_representation();
      this.value.is_member_of = this;
      return this.value;
   }
   
   // If the desired member is the union's active member, return it. Otherwise, 
   // force it to become the union's active member, and then return it. If the 
   // union is internally tagged, then preserve the values of all of its shared 
   // members-of-members.
   //
   // This is used when loading data from a SAV file. Do not use this in a user-
   // defined translator.
   /*CInstance*/ _emplace_for_load(/*Variant<String, CDeclDefinition>*/ member) {
      let common_members = null;
      if (this.type.internal_tag_name) {
         //
         // Keep the internal tag (and all other shared members-of-members).
         //
         if (this.value !== null) {
            if (this.value instanceof CStructInstance && this.value.members[this.type.internal_tag_name]) {
               common_members = {};
               for(let name in this.value.members) {
                  common_members[name] = this.value.members[name];
                  if (name == this.type.tag)
                     break;
               }
            }
         }
      }
      let v = this.#emplace(member);
      if (common_members) {
         for(let name in common_members) {
            v.members[name].copy_contents_of(common_members[name]);
         }
      }
      return v;
   }
};