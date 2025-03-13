class CValueInstance extends CDeclInstance {
   constructor(decl) {
      super(decl);
      
      this.value = null;
      
      let dv = decl.default_value;
      if (dv !== undefined && dv !== null)
         this.value = dv;
      
      this.is_tag_of_unions = []; // Array<CUnionInstance>
   }
   
   copy_contents_of(/*const CValueInstance*/ other) {
      assert_logic(other instanceof CValueInstance,   "Can only copy the contents of a like instance.");
      assert_logic(this.decl.type == other.decl.type, "Can only copy the contents of a like instance.");
      if (other.value === null) {
         this.value = null;
         return;
      }
      if (other.value instanceof DataView) {
         const size   = other.value.byteLength;
         let   buffer = new ArrayBuffer(size);
         this.value = new DataView(buffer);
         
         let i;
         for(i = 0; i < size; i += 4)
            this.value.setUint32(other.value.getUint32(i, true), true);
         for(; i < size; ++i)
            this.value.setUint8(other.value.getUint8(i));
         return;
      }
      if (other.value instanceof PokeString) {
         if (!(this.value instanceof PokeString)) {
            this.value = new PokeString();
         }
         this.value.bytes = ([]).concat(other.value.bytes);
         return;
      }
      this.value = other.value;
   }
}