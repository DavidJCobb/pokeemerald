#ifndef GUARD_LU_SERIALIZE_WonderNewsMetadata
#define GUARD_LU_SERIALIZE_WonderNewsMetadata

struct lu_BitstreamState;
struct WonderNewsMetadata;

void lu_BitstreamRead_WonderNewsMetadata(struct lu_BitstreamState*, struct WonderNewsMetadata* dst);
void lu_BitstreamWrite_WonderNewsMetadata(struct lu_BitstreamState*, const struct WonderNewsMetadata* src);

#endif