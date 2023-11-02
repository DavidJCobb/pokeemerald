#ifndef GUARD_LU_SERIALIZE_PokeNews
#define GUARD_LU_SERIALIZE_PokeNews

struct lu_BitstreamState;
#include "global.h"
#include "global.tv.h"

void lu_BitstreamRead_PokeNews(struct lu_BitstreamState*, PokeNews* dst);
void lu_BitstreamWrite_PokeNews(struct lu_BitstreamState*, const PokeNews* src);

#endif