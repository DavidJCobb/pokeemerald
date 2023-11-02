#ifndef GUARD_LU_SERIALIZE_WonderCard
#define GUARD_LU_SERIALIZE_WonderCard

struct lu_BitstreamState;
struct WonderCard;

void lu_BitstreamRead_WonderCard(struct lu_BitstreamState*, struct WonderCard* dst);
void lu_BitstreamWrite_WonderCard(struct lu_BitstreamState*, const struct WonderCard* src);

#endif