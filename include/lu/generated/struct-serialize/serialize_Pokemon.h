#ifndef GUARD_LU_SERIALIZE_Pokemon
#define GUARD_LU_SERIALIZE_Pokemon

struct lu_BitstreamState;
struct Pokemon;

void lu_BitstreamRead_Pokemon(struct lu_BitstreamState*, struct Pokemon* dst);
void lu_BitstreamWrite_Pokemon(struct lu_BitstreamState*, const struct Pokemon* src);

#endif