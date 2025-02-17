
const FLASH_SECTOR_SIZE = 0x1000;
const FLASH_MEMORY_SIZE = FLASH_SECTOR_SIZE * 32;

const SAVE_SECTOR_FULL_SIZE = FLASH_SECTOR_SIZE;
const SAVE_SECTOR_INFO_SIZE = 128;
const SAVE_SECTOR_DATA_SIZE = SAVE_SECTOR_FULL_SIZE - SAVE_SECTOR_INFO_SIZE;

const SAVE_SECTOR_SIGNATURE = 0x08012025;

const SAVE_SLOT_COUNT       = 2;
const SAVE_SECTORS_PER_SLOT = 14;

function checksum16(blob, length) {
   if (!length)
      length = blob.byteLength;
   let checksum = 0;
   for(let i = 0; i < length; i += 4) {
      checksum += blob.getUint32(i, true);
      checksum |= 0; // constrain to 32-bit int
   }
   checksum = ((checksum >> 16) + checksum) & 0xFFFF;
   return checksum;
}