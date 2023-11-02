#ifndef GUARD_LU_SERIALIZE_ExternalEventFlags
#define GUARD_LU_SERIALIZE_ExternalEventFlags

struct lu_BitstreamState;
struct ExternalEventFlags;

void lu_BitstreamRead_ExternalEventFlags(struct lu_BitstreamState*, struct ExternalEventFlags* dst);
void lu_BitstreamWrite_ExternalEventFlags(struct lu_BitstreamState*, const struct ExternalEventFlags* src);

#endif