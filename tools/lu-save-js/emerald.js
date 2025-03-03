
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

// TODO: Find a clean way to show this stuff in the UI
function get_pokemon_info(/*CStructInstance<SerializedBoxPokemon>*/ inst) {
   let data = {
      gender:    undefined,
      is_shiny:  undefined,
      is_traded: undefined,
      level:     undefined, // as computed based on EXP
      nature:    undefined,
   };
   
   const personality = inst.members.personality.value;
   const trainer_id  = inst.members.otId.value;
   if (personality === null || trainer_id === null)
      return data;
   let slot = inst.save_slot;
   
   {  // Gender
      //
      // We can't compute gender without knowing the gender ratio of 
      // the Pokemon's species, and we don't have that data yet. Once 
      // we do, we'd compute the gender as follows:
      //
      //  - If the ratio is MON_MALE (0), then the Pokemon is always 
      //    male.
      //
      //  - If the ratio is MON_FEMALE, then the Pokemon is always 
      //    female.
      //
      //  - If the ratio is MON_GENDERLESS (255), then the Pokemon is 
      //    always genderless.
      //
      //  - Otherwise, the Pokemon is male if the following expression 
      //    is true, or female otherwise:
      //
      //       (personality & 0xFF) >= species_gender_ratio
      //
   }
   data.is_shiny = (function() {
      const SHINY_ODDS = 8; // include/constants/pokemon.h
      
      return (
         (trainer_id >> 16) ^
         (trainer_id & 0xFFFF) ^
         (personality >> 16) ^
         (personality & 0xFFFF)
      ) < SHINY_ODDS;
   })();
   data.is_traded = (function() {
      let trainer_name = slot.lookupCInstanceByPath("gSaveBlock2Ptr->playerName");
      if (!trainer_name || !trainer_name.value || !(trainer_name.value instanceof PokeString))
         return;
      {
         let a = inst.members.otName.value;
         let b = trainer_name;
         for(let i = 0; i < Math.min(a.length, b.length); ++i) {
            if (a.bytes[i] != b.bytes[i])
               return false;
            if (a.bytes[i] == CHARSET_CONTROL_CODES.chars_to_bytes["\0"])
               break;
         }
      }
      
      let player_id = 0;
      {
         let array = slot.lookupCInstanceByPath("gSaveBlock2Ptr->playerTrainerId");
         if (!array || !(array instanceof CArrayInstance) || array.values.length != 4)
            return;
         for(let i = 0; i < 4; ++i) {
            let v = array.values[i];
            if (v.value === null)
               return;
            player_id |= (v.value << (i * 8));
         }
      }
      if (trainer_id !== player_id)
         return false;
      
      return true;
   })();
   {  // Level
      //
      // We can't compute level without knowing the EXP curve of the 
      // Pokemon's species, and we don't have that data yet.
      //
   }
   {  // Nature
      const NATURES = [ // include/constants/pokemon.h
         "Hardy",
         "Lonely",
         "Brave",
         "Adamant",
         "Naughty",
         "Bold",
         "Docile",
         "Relaxed",
         "Impish",
         "Lax",
         "Timid",
         "Hasty",
         "Serious",
         "Jolly",
         "Naive",
         "Modest",
         "Mild",
         "Quiet",
         "Bashful",
         "Rash",
         "Calm",
         "Gentle",
         "Sassy",
         "Careful",
         "Quirky",
      ];
      data.nature = NATURES[personality % NATURES.length];
   }
   
   
   return data;
}
