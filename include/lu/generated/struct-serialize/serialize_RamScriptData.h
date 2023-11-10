#ifndef GUARD_LU_SERIALIZE_RamScriptData
#define GUARD_LU_SERIALIZE_RamScriptData

struct lu_BitstreamState;
struct RamScriptData;

void lu_BitstreamRead_RamScriptData(struct lu_BitstreamState*, struct RamScriptData* dst);
void lu_BitstreamWrite_RamScriptData(struct lu_BitstreamState*, const struct RamScriptData* src);

#endif