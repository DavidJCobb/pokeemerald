#ifndef GUARD_LU_SERIALIZE_BattleTowerInterview
#define GUARD_LU_SERIALIZE_BattleTowerInterview

struct lu_BitstreamState;
struct BattleTowerInterview;

void lu_BitstreamRead_BattleTowerInterview(struct lu_BitstreamState*, struct BattleTowerInterview* dst);
void lu_BitstreamWrite_BattleTowerInterview(struct lu_BitstreamState*, const struct BattleTowerInterview* src);

#endif