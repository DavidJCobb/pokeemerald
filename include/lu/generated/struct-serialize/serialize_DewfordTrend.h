#ifndef GUARD_LU_SERIALIZE_DewfordTrend
#define GUARD_LU_SERIALIZE_DewfordTrend

struct lu_BitstreamState;
struct DewfordTrend;

void lu_BitstreamRead_DewfordTrend(struct lu_BitstreamState*, struct DewfordTrend* dst);
void lu_BitstreamWrite_DewfordTrend(struct lu_BitstreamState*, const struct DewfordTrend* src);

#endif