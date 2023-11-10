#ifndef GUARD_LU_SERIALIZE_BattleFrontier
#define GUARD_LU_SERIALIZE_BattleFrontier

struct lu_BitstreamState;
struct BattleFrontier;

void lu_BitstreamRead_BattleFrontier(struct lu_BitstreamState*, struct BattleFrontier* dst);
void lu_BitstreamWrite_BattleFrontier(struct lu_BitstreamState*, const struct BattleFrontier* src);

#endif