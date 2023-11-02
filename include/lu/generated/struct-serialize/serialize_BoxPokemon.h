#ifndef GUARD_LU_SERIALIZE_BoxPokemon
#define GUARD_LU_SERIALIZE_BoxPokemon

struct lu_BitstreamState;
struct BoxPokemon;

void lu_BitstreamRead_BoxPokemon(struct lu_BitstreamState*, struct BoxPokemon* dst);
void lu_BitstreamWrite_BoxPokemon(struct lu_BitstreamState*, const struct BoxPokemon* src);

#endif