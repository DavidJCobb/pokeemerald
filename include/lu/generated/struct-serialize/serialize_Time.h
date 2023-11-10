#ifndef GUARD_LU_SERIALIZE_Time
#define GUARD_LU_SERIALIZE_Time

struct lu_BitstreamState;
struct Time;

void lu_BitstreamRead_Time(struct lu_BitstreamState*, struct Time* dst);
void lu_BitstreamWrite_Time(struct lu_BitstreamState*, const struct Time* src);

#endif