#ifndef GUARD_LU_SERIALIZE_Berry2
#define GUARD_LU_SERIALIZE_Berry2

struct lu_BitstreamState;
struct Berry2;

void lu_BitstreamRead_Berry2(struct lu_BitstreamState*, struct Berry2* dst);
void lu_BitstreamWrite_Berry2(struct lu_BitstreamState*, const struct Berry2* src);

#endif