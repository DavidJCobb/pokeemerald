
/*Optional<CStructInstance>*/ function get_enclosing_SerializedBoxPokemon(/*CInstance*/ inst) {
   for(; inst; inst = inst.is_member_of) {
      if (!(inst instanceof CStructInstance))
         continue;
      if (inst instanceof SaveSlot)
         continue;
      if (inst.type.tag == "SerializedBoxPokemon") {
         return inst;
      }
   }
   return null;
}

/*uint16_t*/ function recalc_SerializedBoxPokemon_checksum(/*CStructInstance*/ inst) {
   //
   // The checksum is computed based on the padded substructs, not 
   // just the serialized ones.
   //
   let subs     = inst.members.substructs;
   let size_per = 0;
   for(let name in subs.members) {
      if (name == "checksum")
         continue;
      let memb = subs.members[name];
      if (!(memb instanceof CStructInstance))
         continue;
      size_per = Math.max(size_per, memb.type.c_info.size);
   }
   assert_logic(size_per > 0);
   //
   // First, we need to pack the data as it would've been in memory 
   // during play.
   //
   let buffer = new ArrayBuffer(size_per * 4);
   let view   = new DataView(buffer);
   {
      let offset = 0;
      for(let name in subs.members) {
         let memb = subs.members[name];
         let info = memb.type.c_info;
         {
            let padding = offset % info.alignmnet;
            if (padding) {
               padding = info.alignment - padding;
               offset += padding;
            }
         }
         if (name == "checksum")
            break;
         let buf = new CBufferifier();
         buf.run(memb);
         memcpy(
            new DataView(buffer, offset),
            buf.view,
            buf.view.byteLength
         );
         offset += buf.view.byteLength;
      }
      //
      // NOTE: The game computes the checksum of the decrypted data, 
      // so we do not need to encrypt it here.
      //
   }
   //
   // Now checksum each struct individually. I don't know offhand if 
   // GCC would pad between the structs or within the structs, should 
   // their lengths not equal a multiple of 4; but i do know that the 
   // game itself checksums the structs one-by-one instead of just 
   // checksumming the entire block of memory in which they reside.
   //
   let sum = 0;
   {
      let stride = size_per;
      if (size_per % 4)
         stride += 4 - (size_per % 4);
      for(let i = 0; i < 4; ++i) {
         let offset = stride * i;
         for(let j = 0; j + 1 < size_per; j += 2) {
            sum += view.getUint16(offset + j, true);
            sum &= 0xFFFF;
         }
      }
   }
   return sum;
}
