#ifndef GUARD_LU_SERIALIZE_WonderCardMetadata
#define GUARD_LU_SERIALIZE_WonderCardMetadata

struct lu_BitstreamState;
struct WonderCardMetadata;

void lu_BitstreamRead_WonderCardMetadata(struct lu_BitstreamState*, struct WonderCardMetadata* dst);
void lu_BitstreamWrite_WonderCardMetadata(struct lu_BitstreamState*, const struct WonderCardMetadata* src);

#endif