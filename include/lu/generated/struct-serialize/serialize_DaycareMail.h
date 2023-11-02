#ifndef GUARD_LU_SERIALIZE_DaycareMail
#define GUARD_LU_SERIALIZE_DaycareMail

struct lu_BitstreamState;
struct DaycareMail;

void lu_BitstreamRead_DaycareMail(struct lu_BitstreamState*, struct DaycareMail* dst);
void lu_BitstreamWrite_DaycareMail(struct lu_BitstreamState*, const struct DaycareMail* src);

#endif