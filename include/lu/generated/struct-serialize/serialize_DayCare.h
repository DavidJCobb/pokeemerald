#ifndef GUARD_LU_SERIALIZE_DayCare
#define GUARD_LU_SERIALIZE_DayCare

struct lu_BitstreamState;
struct DayCare;

void lu_BitstreamRead_DayCare(struct lu_BitstreamState*, struct DayCare* dst);
void lu_BitstreamWrite_DayCare(struct lu_BitstreamState*, const struct DayCare* src);

#endif