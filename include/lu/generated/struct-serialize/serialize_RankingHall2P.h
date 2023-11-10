#ifndef GUARD_LU_SERIALIZE_RankingHall2P
#define GUARD_LU_SERIALIZE_RankingHall2P

struct lu_BitstreamState;
struct RankingHall2P;

void lu_BitstreamRead_RankingHall2P(struct lu_BitstreamState*, struct RankingHall2P* dst);
void lu_BitstreamWrite_RankingHall2P(struct lu_BitstreamState*, const struct RankingHall2P* src);

#endif