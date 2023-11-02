#ifndef GUARD_LU_SERIALIZE_TrainerNameRecord
#define GUARD_LU_SERIALIZE_TrainerNameRecord

struct lu_BitstreamState;
struct TrainerNameRecord;

void lu_BitstreamRead_TrainerNameRecord(struct lu_BitstreamState*, struct TrainerNameRecord* dst);
void lu_BitstreamWrite_TrainerNameRecord(struct lu_BitstreamState*, const struct TrainerNameRecord* src);

#endif