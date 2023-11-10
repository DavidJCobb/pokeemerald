#ifndef GUARD_LU_SERIALIZE_EnigmaBerry
#define GUARD_LU_SERIALIZE_EnigmaBerry

struct lu_BitstreamState;
struct EnigmaBerry;

void lu_BitstreamRead_EnigmaBerry(struct lu_BitstreamState*, struct EnigmaBerry* dst);
void lu_BitstreamWrite_EnigmaBerry(struct lu_BitstreamState*, const struct EnigmaBerry* src);

#endif