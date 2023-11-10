#ifndef GUARD_LU_SERIALIZE_LinkBattleRecords
#define GUARD_LU_SERIALIZE_LinkBattleRecords

struct lu_BitstreamState;
struct LinkBattleRecords;

void lu_BitstreamRead_LinkBattleRecords(struct lu_BitstreamState*, struct LinkBattleRecords* dst);
void lu_BitstreamWrite_LinkBattleRecords(struct lu_BitstreamState*, const struct LinkBattleRecords* src);

#endif