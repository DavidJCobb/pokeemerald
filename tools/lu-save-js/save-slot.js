
// For a loaded save slot.
class SaveSlot extends CStructInstance {
   constructor(save_format) {
      super(save_format, null);
      this.sectors     = [];
      this.save_format = save_format;
      if (save_format) {
         if (!(save_format instanceof SaveFormat))
            throw new TypeError("SaveFormat instance or falsy expected");
         for(let tlv of save_format.top_level_values) {
            let value = tlv.make_instance_representation(save_format);
            this.members[tlv.name] = value;
         }
      }
   }
   
   loadSectorMetadata(/*const DataView*/ sector_view) {
      console.assert(sector_view.byteLength == SAVE_SECTOR_FULL_SIZE);
      
      let offset = SAVE_SECTOR_FULL_SIZE - 16;
      let header = {};
      this.sectors.push(header);
      
      function read(bits) {
         let v = sector_view[`getUint${bits}`](offset, true);
         offset += bits / 8;
         return v;
      }
      header.version   = read(32);
      header.sector_id = read(16);
      header.checksum  = read(16);
      header.signature = read(32);
      header.counter   = read(32);
      
      let computed = checksumA32R16(sector_view, SAVE_SECTOR_DATA_SIZE);
      header.checksum_is_valid = computed == header.checksum;
      
      return header;
   }
   
   get counter() {
      let counter = -1;
      for(let sector of this.sectors)
         if (sector.signature == SAVE_SECTOR_SIGNATURE)
            counter = Math.max(counter, sector.counter);
      return counter >= 0 ? counter : null;
   }
   get version() {
      let version = -1;
      for(let sector of this.sectors)
         if (sector.signature == SAVE_SECTOR_SIGNATURE)
            version = Math.max(version, sector.version);
      return version >= 0 ? version : null;
   }
};