#ifndef GUARD_LU_SAVE_TRANSFORM_BoxPokemon
#define GUARD_LU_SAVE_TRANSFORM_BoxPokemon

#include "lu/game_typedefs.h"

struct BoxPokemon;
struct SerializedBoxPokemon;

extern void PackBoxPokemonForSave(const struct BoxPokemon*, struct SerializedBoxPokemon*);
extern void UnpackBoxPokemonForSave(struct BoxPokemon*, const struct SerializedBoxPokemon*);

#endif