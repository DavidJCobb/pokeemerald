#ifndef GUARD_LU_SERIALIZE_BattleTowerPokemon
#define GUARD_LU_SERIALIZE_BattleTowerPokemon

struct lu_BitstreamState;
struct BattleTowerPokemon;

void lu_BitstreamRead_BattleTowerPokemon(struct lu_BitstreamState*, struct BattleTowerPokemon* dst);
void lu_BitstreamWrite_BattleTowerPokemon(struct lu_BitstreamState*, const struct BattleTowerPokemon* src);

#endif