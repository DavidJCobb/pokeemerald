#ifndef GUARD_LU_SERIALIZE_Coords16
#define GUARD_LU_SERIALIZE_Coords16

struct lu_BitstreamState;
struct Coords16;

void lu_BitstreamRead_Coords16(struct lu_BitstreamState*, struct Coords16* dst);
void lu_BitstreamWrite_Coords16(struct lu_BitstreamState*, const struct Coords16* src);

#endif