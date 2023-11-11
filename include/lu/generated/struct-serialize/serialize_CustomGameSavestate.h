#ifndef GUARD_LU_SERIALIZE_CustomGameSavestate
#define GUARD_LU_SERIALIZE_CustomGameSavestate

struct lu_BitstreamState;
struct CustomGameSavestate;

void lu_BitstreamRead_CustomGameSavestate(struct lu_BitstreamState*, struct CustomGameSavestate* dst);
void lu_BitstreamWrite_CustomGameSavestate(struct lu_BitstreamState*, const struct CustomGameSavestate* src);

#endif