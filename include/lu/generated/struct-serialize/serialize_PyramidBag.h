#ifndef GUARD_LU_SERIALIZE_PyramidBag
#define GUARD_LU_SERIALIZE_PyramidBag

struct lu_BitstreamState;
struct PyramidBag;

void lu_BitstreamRead_PyramidBag(struct lu_BitstreamState*, struct PyramidBag* dst);
void lu_BitstreamWrite_PyramidBag(struct lu_BitstreamState*, const struct PyramidBag* src);

#endif