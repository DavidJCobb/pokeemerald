#ifndef GUARD_LU_SERIALIZE_ObjectEventTemplate
#define GUARD_LU_SERIALIZE_ObjectEventTemplate

struct lu_BitstreamState;
struct ObjectEventTemplate;

void lu_BitstreamRead_ObjectEventTemplate(struct lu_BitstreamState*, struct ObjectEventTemplate* dst);
void lu_BitstreamWrite_ObjectEventTemplate(struct lu_BitstreamState*, const struct ObjectEventTemplate* src);

#endif