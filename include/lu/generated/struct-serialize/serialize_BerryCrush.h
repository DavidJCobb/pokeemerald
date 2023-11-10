#ifndef GUARD_LU_SERIALIZE_BerryCrush
#define GUARD_LU_SERIALIZE_BerryCrush

struct lu_BitstreamState;
struct BerryCrush;

void lu_BitstreamRead_BerryCrush(struct lu_BitstreamState*, struct BerryCrush* dst);
void lu_BitstreamWrite_BerryCrush(struct lu_BitstreamState*, const struct BerryCrush* src);

#endif