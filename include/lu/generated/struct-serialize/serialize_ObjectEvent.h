#ifndef GUARD_LU_SERIALIZE_ObjectEvent
#define GUARD_LU_SERIALIZE_ObjectEvent

struct lu_BitstreamState;
struct ObjectEvent;

void lu_BitstreamRead_ObjectEvent(struct lu_BitstreamState*, struct ObjectEvent* dst);
void lu_BitstreamWrite_ObjectEvent(struct lu_BitstreamState*, const struct ObjectEvent* src);

#endif