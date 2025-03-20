
class SaveFile {
   #save_format;
   
   constructor(save_format) {
      this.#save_format = save_format;
      this.slots = []; // Array<SaveSlot>
      this.special_sectors = {
         hall_of_fame:    [ null, null ], // Array<DataView>
         trainer_hill:    null, // DataView
         recorded_battle: null, // DataView
      };
      this.rtc = null; // Optional<std::array<uint8_t,0x10>>
      
      for(let i = 0; i < SAVE_SLOT_COUNT; ++i) {
         this.slots[i] = new SaveSlot(this);
      }
   }
   
   get save_format() { return this.#save_format; }
   get version() {
      let slot = this.current_slot;
      if (slot)
         return slot.version;
   }
   
   get current_slot() {
      let counter = -1;
      let slot    = null;
      for(let s of this.slots) {
         let c = s.counter;
         if (c > counter) {
            counter = c;
            slot    = s;
         }
      }
      return slot;
   }
};