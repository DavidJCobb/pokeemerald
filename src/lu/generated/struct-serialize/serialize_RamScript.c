#include "lu/generated/struct-serialize//serialize_RamScript.h"

#include "global.h"

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_RamScriptData.h"

void lu_BitstreamRead_RamScript(struct lu_BitstreamState* state, struct RamScript* v) {
   v->checksum = lu_BitstreamRead_u32(state, 32);
   lu_BitstreamRead_RamScriptData(state, &v->data);
}

void lu_BitstreamWrite_RamScript(struct lu_BitstreamState* state, const struct RamScript* v) {
   lu_BitstreamWrite_u32(state, v->checksum, 32);
   lu_BitstreamWrite_RamScriptData(state, &v->data);
}
