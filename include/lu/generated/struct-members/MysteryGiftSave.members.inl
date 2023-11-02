   u32 newsCrc;
   struct WonderNews news;
   u32 cardCrc;
   struct WonderCard card;
   u32 cardMetadataCrc;
   struct WonderCardMetadata cardMetadata;
   u16 questionnaireWords[NUM_QUESTIONNAIRE_WORDS];
   struct WonderNewsMetadata newsMetadata;
   u32 trainerIds[2][5]; // Saved ids for 10 trainers, 5 each for battles and trades
