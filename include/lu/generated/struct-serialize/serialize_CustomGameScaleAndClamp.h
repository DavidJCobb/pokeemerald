#ifndef GUARD_LU_SERIALIZE_CustomGameScaleAndClamp
#define GUARD_LU_SERIALIZE_CustomGameScaleAndClamp

struct lu_BitstreamState;
struct CustomGameScaleAndClamp;

void lu_BitstreamRead_CustomGameScaleAndClamp(struct lu_BitstreamState*, struct CustomGameScaleAndClamp* dst);
void lu_BitstreamWrite_CustomGameScaleAndClamp(struct lu_BitstreamState*, const struct CustomGameScaleAndClamp* src);

#endif