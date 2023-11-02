#ifndef GUARD_LU_SERIALIZE_BattleDomeTrainer
#define GUARD_LU_SERIALIZE_BattleDomeTrainer

struct lu_BitstreamState;
struct BattleDomeTrainer;

void lu_BitstreamRead_BattleDomeTrainer(struct lu_BitstreamState*, struct BattleDomeTrainer* dst);
void lu_BitstreamWrite_BattleDomeTrainer(struct lu_BitstreamState*, const struct BattleDomeTrainer* src);

#endif