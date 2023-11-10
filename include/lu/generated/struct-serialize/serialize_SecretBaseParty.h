#ifndef GUARD_LU_SERIALIZE_SecretBaseParty
#define GUARD_LU_SERIALIZE_SecretBaseParty

struct lu_BitstreamState;
struct SecretBaseParty;

void lu_BitstreamRead_SecretBaseParty(struct lu_BitstreamState*, struct SecretBaseParty* dst);
void lu_BitstreamWrite_SecretBaseParty(struct lu_BitstreamState*, const struct SecretBaseParty* src);

#endif