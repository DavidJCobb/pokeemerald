#ifndef GUARD_LU_SERIALIZE_ItemSlot
#define GUARD_LU_SERIALIZE_ItemSlot

struct lu_BitstreamState;
struct ItemSlot;

void lu_BitstreamRead_ItemSlot(struct lu_BitstreamState*, struct ItemSlot* dst);
void lu_BitstreamWrite_ItemSlot(struct lu_BitstreamState*, const struct ItemSlot* src);

#endif