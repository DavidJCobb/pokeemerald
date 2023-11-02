#ifndef GUARD_LU_SERIALIZE_Roamer
#define GUARD_LU_SERIALIZE_Roamer

struct lu_BitstreamState;
struct Roamer;

void lu_BitstreamRead_Roamer(struct lu_BitstreamState*, struct Roamer* dst);
void lu_BitstreamWrite_Roamer(struct lu_BitstreamState*, const struct Roamer* src);

#endif