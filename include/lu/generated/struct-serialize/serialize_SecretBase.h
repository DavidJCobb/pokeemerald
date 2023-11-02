#ifndef GUARD_LU_SERIALIZE_SecretBase
#define GUARD_LU_SERIALIZE_SecretBase

struct lu_BitstreamState;
struct SecretBase;

void lu_BitstreamRead_SecretBase(struct lu_BitstreamState*, struct SecretBase* dst);
void lu_BitstreamWrite_SecretBase(struct lu_BitstreamState*, const struct SecretBase* src);

#endif