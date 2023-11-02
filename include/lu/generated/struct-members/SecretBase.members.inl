   u8 secretBaseId;
   bool8 toRegister : 4;
   u8 gender : 1;
   u8 battledOwnerToday : 1;
   u8 registryStatus : 2;
   u8 trainerName[PLAYER_NAME_LENGTH];
   u8 trainerId[TRAINER_ID_LENGTH]; // byte 0 is used for determining trainer class
   u8 language;
   u16 numSecretBasesReceived;
   u8 numTimesEntered;
   u8 unused;
   u8 decorations[DECOR_MAX_SECRET_BASE];
   u8 decorationPositions[DECOR_MAX_SECRET_BASE];
   struct SecretBaseParty party;
