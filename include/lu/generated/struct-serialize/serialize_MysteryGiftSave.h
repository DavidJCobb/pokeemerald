#ifndef GUARD_LU_SERIALIZE_MysteryGiftSave
#define GUARD_LU_SERIALIZE_MysteryGiftSave

struct lu_BitstreamState;
struct MysteryGiftSave;

void lu_BitstreamRead_MysteryGiftSave(struct lu_BitstreamState*, struct MysteryGiftSave* dst);
void lu_BitstreamWrite_MysteryGiftSave(struct lu_BitstreamState*, const struct MysteryGiftSave* src);

#endif