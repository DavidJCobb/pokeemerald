#ifndef GUARD_LU_SERIALIZE_CustomGameOptions
#define GUARD_LU_SERIALIZE_CustomGameOptions

struct lu_BitstreamState;
struct CustomGameOptions;

void lu_BitstreamRead_CustomGameOptions(struct lu_BitstreamState*, struct CustomGameOptions* dst);
void lu_BitstreamWrite_CustomGameOptions(struct lu_BitstreamState*, const struct CustomGameOptions* src);

#endif