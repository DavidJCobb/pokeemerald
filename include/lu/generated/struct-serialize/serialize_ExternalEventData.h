#ifndef GUARD_LU_SERIALIZE_ExternalEventData
#define GUARD_LU_SERIALIZE_ExternalEventData

struct lu_BitstreamState;
struct ExternalEventData;

void lu_BitstreamRead_ExternalEventData(struct lu_BitstreamState*, struct ExternalEventData* dst);
void lu_BitstreamWrite_ExternalEventData(struct lu_BitstreamState*, const struct ExternalEventData* src);

#endif