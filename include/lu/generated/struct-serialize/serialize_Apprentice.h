#ifndef GUARD_LU_SERIALIZE_Apprentice
#define GUARD_LU_SERIALIZE_Apprentice

struct lu_BitstreamState;
struct Apprentice;

void lu_BitstreamRead_Apprentice(struct lu_BitstreamState*, struct Apprentice* dst);
void lu_BitstreamWrite_Apprentice(struct lu_BitstreamState*, const struct Apprentice* src);

#endif