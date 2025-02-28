
// For a loaded save slot.
class SaveSlot extends CStructInstance {
   #save_format;
   
   constructor(/*SaveFormat*/ save_format) {
      assert_type(save_format instanceof SaveFormat);
      super(/*CStructDefinition*/ null);
      this.sectors      = [];
      this.#save_format = save_format;
      for(let tlv of this.save_format.top_level_values) {
         let value = tlv.make_instance_representation();
         value.is_member_of = this;
         this.members[tlv.name] = value;
      }
   }
   
   get save_format() {
      return this.#save_format;
   }
   set save_format(v) {
      if (v)
         assert_type(v instanceof SaveFormat);
      this.#save_format = v;
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
      
      header.raw_sector = null;
      if (header.signature != SAVE_SECTOR_SIGNATURE) {
         //
         // HACK: Save the raw sector content for later export, so that 
         // round-tripped SAV files are byte-identical.
         //
         header.raw_sector = sector_view.buffer.slice(
            sector_view.byteOffset,
            sector_view.byteOffset + SAVE_SECTOR_FULL_SIZE
         );
      }
      
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