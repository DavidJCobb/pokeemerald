#ifndef GUARD_LU_SERIALIZE_WaldaPhrase
#define GUARD_LU_SERIALIZE_WaldaPhrase

struct lu_BitstreamState;
struct WaldaPhrase;

void lu_BitstreamRead_WaldaPhrase(struct lu_BitstreamState*, struct WaldaPhrase* dst);
void lu_BitstreamWrite_WaldaPhrase(struct lu_BitstreamState*, const struct WaldaPhrase* src);

#endif