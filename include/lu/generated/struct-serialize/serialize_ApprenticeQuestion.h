#ifndef GUARD_LU_SERIALIZE_ApprenticeQuestion
#define GUARD_LU_SERIALIZE_ApprenticeQuestion

struct lu_BitstreamState;
struct ApprenticeQuestion;

void lu_BitstreamRead_ApprenticeQuestion(struct lu_BitstreamState*, struct ApprenticeQuestion* dst);
void lu_BitstreamWrite_ApprenticeQuestion(struct lu_BitstreamState*, const struct ApprenticeQuestion* src);

#endif