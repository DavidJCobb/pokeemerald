#ifndef GUARD_LU_SERIALIZE_WonderNews
#define GUARD_LU_SERIALIZE_WonderNews

struct lu_BitstreamState;
struct WonderNews;

void lu_BitstreamRead_WonderNews(struct lu_BitstreamState*, struct WonderNews* dst);
void lu_BitstreamWrite_WonderNews(struct lu_BitstreamState*, const struct WonderNews* src);

#endif