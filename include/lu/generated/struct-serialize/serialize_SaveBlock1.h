#ifndef GUARD_LU_SERIALIZE_SaveBlock1
#define GUARD_LU_SERIALIZE_SaveBlock1

struct lu_BitstreamState;
struct SaveBlock1;

void lu_BitstreamRead_SaveBlock1(struct lu_BitstreamState*, struct SaveBlock1* dst);
void lu_BitstreamWrite_SaveBlock1(struct lu_BitstreamState*, const struct SaveBlock1* src);

#endif