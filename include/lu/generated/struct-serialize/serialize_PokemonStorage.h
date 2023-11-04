#ifndef GUARD_LU_SERIALIZE_PokemonStorage
#define GUARD_LU_SERIALIZE_PokemonStorage

struct lu_BitstreamState;
struct PokemonStorage;

void lu_BitstreamRead_PokemonStorage(struct lu_BitstreamState*, struct PokemonStorage* dst);
void lu_BitstreamWrite_PokemonStorage(struct lu_BitstreamState*, const struct PokemonStorage* src);

#endif