class CArrayInstance extends CDeclInstance {
   constructor(/*CDeclDefinition*/ decl, /*size_t*/ rank = 0) {
      super(decl);
      this.values = null;
      this.rank   = rank || 0;
      
      console.assert(rank < decl.array_extents.length);
      const extent = this.declared_length;
      
      this.values = [];
      for(let i = 0; i < extent; ++i)
         this.#rebuild_element(i);
   }
   
   get declared_length() {
      return this.decl.array_extents[this.rank];
   }
   get is_innermost() {
      return this.rank + 1 == this.decl.array_extents.length;
   }
   
   #rebuild_element(i) {
      let v;
      if (this.is_innermost) {
         v = this.decl.make_instance_representation(true);
      } else {
         v = new CArrayInstance(this.decl, this.rank + 1);
      }
      v.is_member_of = this;
      this.values[i] = v;
   }
   
   copy_contents_of(/*const CArrayInstance*/ other) {
      assert_logic(other instanceof CArrayInstance,           "Can only copy the contents of a like instance.");
      assert_logic(this.decl.type == other.decl.type,         "Can only copy the contents of a like instance.");
      assert_logic(this.values.length == other.values.length, "Can only copy the contents of a like instance.");
      for(let i = 0; i < this.values.length; ++i) {
         this.values[i].copy_contents_of(other.values[i]);
      }
   }
};