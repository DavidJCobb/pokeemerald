#ifndef GUARD_LU_SERIALIZE_ApprenticeMon
#define GUARD_LU_SERIALIZE_ApprenticeMon

struct lu_BitstreamState;
struct ApprenticeMon;

void lu_BitstreamRead_ApprenticeMon(struct lu_BitstreamState*, struct ApprenticeMon* dst);
void lu_BitstreamWrite_ApprenticeMon(struct lu_BitstreamState*, const struct ApprenticeMon* src);

#endif