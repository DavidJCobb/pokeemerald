   u32 personality;
   u32 otId;
   u8 nickname[POKEMON_NAME_LENGTH];
   u8 language;
   u8 isBadEgg : 1;
   u8 hasSpecies : 1;
   u8 isEgg : 1;
   u8 unused : 5;
   u8 otName[PLAYER_NAME_LENGTH];
   u8 markings;
   u16 checksum;
   u16 unknown;
   union {
   u32 raw[(NUM_SUBSTRUCT_BYTES * 4) / 4]; // *4 because there are 4 substructs, /4 because it's u32, not u8
   union PokemonSubstruct substructs[4];
} secure;
