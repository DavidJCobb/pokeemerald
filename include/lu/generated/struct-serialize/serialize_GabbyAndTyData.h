#ifndef GUARD_LU_SERIALIZE_GabbyAndTyData
#define GUARD_LU_SERIALIZE_GabbyAndTyData

struct lu_BitstreamState;
struct GabbyAndTyData;

void lu_BitstreamRead_GabbyAndTyData(struct lu_BitstreamState*, struct GabbyAndTyData* dst);
void lu_BitstreamWrite_GabbyAndTyData(struct lu_BitstreamState*, const struct GabbyAndTyData* src);

#endif