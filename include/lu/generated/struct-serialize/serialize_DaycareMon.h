#ifndef GUARD_LU_SERIALIZE_DaycareMon
#define GUARD_LU_SERIALIZE_DaycareMon

struct lu_BitstreamState;
struct DaycareMon;

void lu_BitstreamRead_DaycareMon(struct lu_BitstreamState*, struct DaycareMon* dst);
void lu_BitstreamWrite_DaycareMon(struct lu_BitstreamState*, const struct DaycareMon* src);

#endif