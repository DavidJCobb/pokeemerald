#ifndef GUARD_LU_SERIALIZE_WarpData
#define GUARD_LU_SERIALIZE_WarpData

struct lu_BitstreamState;
struct WarpData;

void lu_BitstreamRead_WarpData(struct lu_BitstreamState*, struct WarpData* dst);
void lu_BitstreamWrite_WarpData(struct lu_BitstreamState*, const struct WarpData* src);

#endif