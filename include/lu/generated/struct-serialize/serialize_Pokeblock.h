#ifndef GUARD_LU_SERIALIZE_Pokeblock
#define GUARD_LU_SERIALIZE_Pokeblock

struct lu_BitstreamState;
struct Pokeblock;

void lu_BitstreamRead_Pokeblock(struct lu_BitstreamState*, struct Pokeblock* dst);
void lu_BitstreamWrite_Pokeblock(struct lu_BitstreamState*, const struct Pokeblock* src);

#endif