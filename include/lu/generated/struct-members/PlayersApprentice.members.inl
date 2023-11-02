   u8 id;
   u8 lvlMode : 2; // 0: Unassigned, 1: Lv 50, 2: Open Lv
   u8 questionsAnswered : 4;
   u8 leadMonId : 2;
   u8 party : 3;
   u8 saveId : 2;
   u8 unused;
   u8 speciesIds[MULTI_PARTY_SIZE];
   struct ApprenticeQuestion questions[APPRENTICE_MAX_QUESTIONS];
