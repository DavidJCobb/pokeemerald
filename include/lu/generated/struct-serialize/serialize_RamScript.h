#ifndef GUARD_LU_SERIALIZE_RamScript
#define GUARD_LU_SERIALIZE_RamScript

struct lu_BitstreamState;
struct RamScript;

void lu_BitstreamRead_RamScript(struct lu_BitstreamState*, struct RamScript* dst);
void lu_BitstreamWrite_RamScript(struct lu_BitstreamState*, const struct RamScript* src);

#endif