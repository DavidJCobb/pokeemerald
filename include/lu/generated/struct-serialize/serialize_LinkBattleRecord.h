#ifndef GUARD_LU_SERIALIZE_LinkBattleRecord
#define GUARD_LU_SERIALIZE_LinkBattleRecord

struct lu_BitstreamState;
struct LinkBattleRecord;

void lu_BitstreamRead_LinkBattleRecord(struct lu_BitstreamState*, struct LinkBattleRecord* dst);
void lu_BitstreamWrite_LinkBattleRecord(struct lu_BitstreamState*, const struct LinkBattleRecord* src);

#endif