#ifndef GUARD_LU_SERIALIZE_TrainerHillSave
#define GUARD_LU_SERIALIZE_TrainerHillSave

struct lu_BitstreamState;
struct TrainerHillSave;

void lu_BitstreamRead_TrainerHillSave(struct lu_BitstreamState*, struct TrainerHillSave* dst);
void lu_BitstreamWrite_TrainerHillSave(struct lu_BitstreamState*, const struct TrainerHillSave* src);

#endif