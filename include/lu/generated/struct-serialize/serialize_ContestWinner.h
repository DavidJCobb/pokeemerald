#ifndef GUARD_LU_SERIALIZE_ContestWinner
#define GUARD_LU_SERIALIZE_ContestWinner

struct lu_BitstreamState;
struct ContestWinner;

void lu_BitstreamRead_ContestWinner(struct lu_BitstreamState*, struct ContestWinner* dst);
void lu_BitstreamWrite_ContestWinner(struct lu_BitstreamState*, const struct ContestWinner* src);

#endif