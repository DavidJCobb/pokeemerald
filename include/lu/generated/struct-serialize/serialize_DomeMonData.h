#ifndef GUARD_LU_SERIALIZE_DomeMonData
#define GUARD_LU_SERIALIZE_DomeMonData

struct lu_BitstreamState;
struct DomeMonData;

void lu_BitstreamRead_DomeMonData(struct lu_BitstreamState*, struct DomeMonData* dst);
void lu_BitstreamWrite_DomeMonData(struct lu_BitstreamState*, const struct DomeMonData* src);

#endif