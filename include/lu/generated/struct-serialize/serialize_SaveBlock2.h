#ifndef GUARD_LU_SERIALIZE_SaveBlock2
#define GUARD_LU_SERIALIZE_SaveBlock2

struct lu_BitstreamState;
struct SaveBlock2;

void lu_BitstreamRead_SaveBlock2(struct lu_BitstreamState*, struct SaveBlock2* dst);
void lu_BitstreamWrite_SaveBlock2(struct lu_BitstreamState*, const struct SaveBlock2* src);

#endif