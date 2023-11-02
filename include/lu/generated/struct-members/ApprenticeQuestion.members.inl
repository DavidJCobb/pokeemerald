   u8 questionId : 2;
   u8 monId : 2;
   u8 moveSlot : 2;
   u8 suggestedChange : 2; // TRUE if told to use held item or second move, FALSE if told to use no item or first move
   u16 data; // used both as an itemId and a moveId
