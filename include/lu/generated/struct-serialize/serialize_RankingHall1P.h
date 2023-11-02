#ifndef GUARD_LU_SERIALIZE_RankingHall1P
#define GUARD_LU_SERIALIZE_RankingHall1P

struct lu_BitstreamState;
struct RankingHall1P;

void lu_BitstreamRead_RankingHall1P(struct lu_BitstreamState*, struct RankingHall1P* dst);
void lu_BitstreamWrite_RankingHall1P(struct lu_BitstreamState*, const struct RankingHall1P* src);

#endif