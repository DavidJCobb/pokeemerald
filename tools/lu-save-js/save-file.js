
class SaveFile {
   constructor() {
      this.slots = []; // Array<SaveSlot>
      this.special_sectors = {
         hall_of_fame:    [ null, null ], // Array<DataView>
         trainer_hill:    null, // DataView
         recorded_battle: null, // DataView
      };
      this.rtc = null; // Optional<std::array<uint8_t,0x10>>
   }
};