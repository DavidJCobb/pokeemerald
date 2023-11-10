#ifndef GUARD_LU_SERIALIZE_Mail
#define GUARD_LU_SERIALIZE_Mail

struct lu_BitstreamState;
struct Mail;

void lu_BitstreamRead_Mail(struct lu_BitstreamState*, struct Mail* dst);
void lu_BitstreamWrite_Mail(struct lu_BitstreamState*, const struct Mail* src);

#endif