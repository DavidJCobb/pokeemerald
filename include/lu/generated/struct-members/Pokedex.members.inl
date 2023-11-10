   u8 order;
   u8 mode;
   u8 nationalMagic; // must equal 0xDA in order to have National Mode
   u8 unknown2;
   u32 unownPersonality; // set when you first see Unown
   u32 spindaPersonality; // set when you first see Spinda
   u32 unknown3;
   u8 owned[NUM_DEX_FLAG_BYTES];
   u8 seen[NUM_DEX_FLAG_BYTES];
