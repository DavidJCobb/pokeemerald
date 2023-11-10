#ifndef GUARD_LU_SERIALIZE_BerryPickingResults
#define GUARD_LU_SERIALIZE_BerryPickingResults

struct lu_BitstreamState;
struct BerryPickingResults;

void lu_BitstreamRead_BerryPickingResults(struct lu_BitstreamState*, struct BerryPickingResults* dst);
void lu_BitstreamWrite_BerryPickingResults(struct lu_BitstreamState*, const struct BerryPickingResults* src);

#endif