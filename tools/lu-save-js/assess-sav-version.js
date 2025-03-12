/*Optional<int>*/ function assess_sav_version(/*DataView*/ sav) {
   if (sav.byteLength < FLASH_MEMORY_SIZE) {
      return null;
   }
   let max_versions = [];
   let max_counter  = -1;
   let current_slot = -1;
   for(let i = 0; i < SAVE_SLOT_COUNT; ++i) {
      for(let j = 0; j < SAVE_SECTORS_PER_SLOT; ++j) {
         let pos  = FLASH_SECTOR_SIZE * (SAVE_SECTORS_PER_SLOT * i + j);
         let blob = new DataView(sav.buffer, pos, FLASH_SECTOR_SIZE);
         
         let k = FLASH_SECTOR_SIZE - 16;
         let version   = blob.getUint32(k,      true);
         let signature = blob.getUint32(k +  8, true);
         let counter   = blob.getUint32(k + 12, true);
         if (signature != SAVE_SECTOR_SIGNATURE) {
            continue;
         }
         if (!max_versions[i] || version > max_versions[i]) {
            max_versions[i] = version;
         }
         if (counter > max_counter) {
            max_counter  = counter;
            current_slot = i;
         }
      }
   }
   if (current_slot < 0) {
      return null;
   }
   return max_versions[current_slot];
}