#ifndef GUARD_LU_SERIALIZE_BerryTree
#define GUARD_LU_SERIALIZE_BerryTree

struct lu_BitstreamState;
struct BerryTree;

void lu_BitstreamRead_BerryTree(struct lu_BitstreamState*, struct BerryTree* dst);
void lu_BitstreamWrite_BerryTree(struct lu_BitstreamState*, const struct BerryTree* src);

#endif