#ifndef GUARD_LU_SERIALIZE_PlayersApprentice
#define GUARD_LU_SERIALIZE_PlayersApprentice

struct lu_BitstreamState;
struct PlayersApprentice;

void lu_BitstreamRead_PlayersApprentice(struct lu_BitstreamState*, struct PlayersApprentice* dst);
void lu_BitstreamWrite_PlayersApprentice(struct lu_BitstreamState*, const struct PlayersApprentice* src);

#endif