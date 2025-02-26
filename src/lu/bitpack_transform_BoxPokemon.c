#include "lu/bitpack_transform_BoxPokemon.h"
#include "lu/bitpack_transformed_type_BoxPokemon.h"

static const u8 scrambled_substruct_indices[24][4] = {
   { 0, 1, 2, 3 }, 
   { 0, 1, 3, 2 }, 
   { 0, 2, 1, 3 }, 
   { 0, 3, 1, 2 }, 
   { 0, 2, 3, 1 }, 
   { 0, 3, 2, 1 }, 
   { 1, 0, 2, 3 }, 
   { 1, 0, 3, 2 }, 
   { 2, 0, 1, 3 }, 
   { 3, 0, 1, 2 }, 
   { 2, 0, 3, 1 }, 
   { 3, 0, 2, 1 }, 
   { 1, 2, 0, 3 }, 
   { 1, 3, 0, 2 }, 
   { 2, 1, 0, 3 }, 
   { 3, 1, 0, 2 }, 
   { 2, 3, 0, 1 }, 
   { 3, 2, 0, 1 }, 
   { 1, 2, 3, 0 }, 
   { 1, 3, 2, 0 }, 
   { 2, 1, 3, 0 }, 
   { 3, 1, 2, 0 }, 
   { 2, 3, 1, 0 }, 
   { 3, 2, 1, 0 },
};

typedef u32 substruct_buffer [(NUM_SUBSTRUCT_BYTES * 4) / 4];
typedef union PokemonSubstruct substruct_array [4];

static union PokemonSubstruct* GetSubstruct(u32 personality, union PokemonSubstruct* substructs, u8 which) {
   u8 scramble = personality % 24;
   return &substructs[scrambled_substruct_indices[scramble][which]];
}

static void PackSubstructs(u32 personality, const union PokemonSubstruct* src, struct SerializedBoxPokemonSubstructs* dst) {
   u8 scramble = personality % 24;
   memcpy(
      &dst->type0,
      &src[scrambled_substruct_indices[scramble][0]],
      sizeof(struct PokemonSubstruct0)
   );
   memcpy(
      &dst->type1,
      &src[scrambled_substruct_indices[scramble][1]],
      sizeof(struct PokemonSubstruct1)
   );
   memcpy(
      &dst->type2,
      &src[scrambled_substruct_indices[scramble][2]],
      sizeof(struct PokemonSubstruct2)
   );
   memcpy(
      &dst->type3,
      &src[scrambled_substruct_indices[scramble][3]],
      sizeof(struct PokemonSubstruct3)
   );
}
static void UnpackSubstructs(u32 personality, union PokemonSubstruct* src, const struct SerializedBoxPokemonSubstructs* dst) {
   u8 scramble = personality % 24;
   memcpy(
      &src[scrambled_substruct_indices[scramble][0]],
      &dst->type0,
      sizeof(struct PokemonSubstruct0)
   );
   memcpy(
      &src[scrambled_substruct_indices[scramble][1]],
      &dst->type1,
      sizeof(struct PokemonSubstruct1)
   );
   memcpy(
      &src[scrambled_substruct_indices[scramble][2]],
      &dst->type2,
      sizeof(struct PokemonSubstruct2)
   );
   memcpy(
      &src[scrambled_substruct_indices[scramble][3]],
      &dst->type3,
      sizeof(struct PokemonSubstruct3)
   );
}

extern void PackBoxPokemonForSave(const struct BoxPokemon* src, struct SerializedBoxPokemon* dst) {
   dst->personality = src->personality;
   dst->otId        = src->otId;
   for(u32 i = 0; i < POKEMON_NAME_LENGTH; ++i) {
      dst->nickname[i] = src->nickname[i];
   }
   dst->language    = src->language;
   dst->isBadEgg    = src->isBadEgg;
   dst->hasSpecies  = src->hasSpecies;
   dst->isEgg       = src->isEgg;
   dst->blockBoxRS  = src->blockBoxRS;
   for(u32 i = 0; i < PLAYER_NAME_LENGTH; ++i) {
      dst->otName[i] = src->otName[i];
   }
   dst->markings    = src->markings;
   dst->substructs.checksum = src->checksum;
   dst->unknown     = src->unknown;
   
   //
   // Decrypt the substruct data. (Same implementation as DecryptBoxMon, 
   // internal to pokemon.c.)
   //
   union {
      u32 raw[(NUM_SUBSTRUCT_BYTES * 4) / 4];
      union PokemonSubstruct substructs[4];
   } decrypted;
   for (u32 i = 0; i < ARRAY_COUNT(src->secure.raw); i++) {
      u32 dword = src->secure.raw[i];
      dword ^= src->otId;
      dword ^= src->personality;
      decrypted.raw[i] = dword;
   }
   PackSubstructs(src->personality, &decrypted.substructs[0], &dst->substructs);
}
extern void UnpackBoxPokemonForSave(struct BoxPokemon* dst, const struct SerializedBoxPokemon* src) {
   ZeroBoxMonData(dst);
   
   dst->personality = src->personality;
   dst->otId        = src->otId;
   for(u32 i = 0; i < POKEMON_NAME_LENGTH; ++i) {
      dst->nickname[i] = src->nickname[i];
   }
   dst->language    = src->language;
   dst->isBadEgg    = src->isBadEgg;
   dst->hasSpecies  = src->hasSpecies;
   dst->isEgg       = src->isEgg;
   dst->blockBoxRS  = src->blockBoxRS;
   for(u32 i = 0; i < PLAYER_NAME_LENGTH; ++i) {
      dst->otName[i] = src->otName[i];
   }
   dst->markings    = src->markings;
   dst->checksum    = src->substructs.checksum;
   dst->unknown     = src->unknown;
   
   UnpackSubstructs(src->personality, &dst->secure.substructs[0], &src->substructs);
   //
   // Encrypt the substruct data. (Same implementation as EncryptBoxMon, 
   // internal to pokemon.c.)
   //
   for (u32 i = 0; i < ARRAY_COUNT(dst->secure.raw); i++) {
      dst->secure.raw[i] ^= dst->personality;
      dst->secure.raw[i] ^= dst->otId;
   }
}