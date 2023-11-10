#ifndef GUARD_LU_SERIALIZE_PokemonJumpRecords
#define GUARD_LU_SERIALIZE_PokemonJumpRecords

struct lu_BitstreamState;
struct PokemonJumpRecords;

void lu_BitstreamRead_PokemonJumpRecords(struct lu_BitstreamState*, struct PokemonJumpRecords* dst);
void lu_BitstreamWrite_PokemonJumpRecords(struct lu_BitstreamState*, const struct PokemonJumpRecords* src);

#endif