   u16 flagId; // Event flag (sReceivedGiftFlags) + WONDER_CARD_FLAG_OFFSET
   u16 iconSpecies;
   u32 idNumber;
   u8 type : 2; // CARD_TYPE_*
   u8 bgType : 4;
   u8 sendType : 2; // SEND_TYPE_*
   u8 maxStamps;
   u8 titleText[WONDER_CARD_TEXT_LENGTH];
   u8 subtitleText[WONDER_CARD_TEXT_LENGTH];
   u8 bodyText[WONDER_CARD_BODY_TEXT_LINES][WONDER_CARD_TEXT_LENGTH];
   u8 footerLine1Text[WONDER_CARD_TEXT_LENGTH];
   u8 footerLine2Text[WONDER_CARD_TEXT_LENGTH];
