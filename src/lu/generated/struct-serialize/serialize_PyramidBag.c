#include "lu/generated/struct-serialize//serialize_PyramidBag.h"

#include "global.h"

#include "lu/bitstreams.h"

// check constants:
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PYRAMID_BAG_ITEMS_COUNT != 10
   #error Constant `PYRAMID_BAG_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if FRONTIER_LVL_MODE_COUNT != 2
   #error Constant `FRONTIER_LVL_MODE_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif
#if PYRAMID_BAG_ITEMS_COUNT != 10
   #error Constant `PYRAMID_BAG_ITEMS_COUNT` changed in C, but XML not updated or codegen not re-run!
#endif

void lu_BitstreamRead_PyramidBag(struct lu_BitstreamState* state, struct PyramidBag* v) {
   u8 i, j;
   for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) {
      for (j = 0; j < PYRAMID_BAG_ITEMS_COUNT; ++j) {
         v->itemId[i][j] = lu_BitstreamRead_u16(state, 9);
      }
   }
   for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) {
      for (j = 0; j < PYRAMID_BAG_ITEMS_COUNT; ++j) {
         v->quantity[i][j] = lu_BitstreamRead_u8(state, 7);
      }
   }
}

void lu_BitstreamWrite_PyramidBag(struct lu_BitstreamState* state, const struct PyramidBag* v) {
   u8 i, j;
   for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) {
      for (j = 0; j < PYRAMID_BAG_ITEMS_COUNT; ++j) {
         lu_BitstreamWrite_u16(state, v->itemId[i][j], 9);
      }
   }
   for (i = 0; i < FRONTIER_LVL_MODE_COUNT; ++i) {
      for (j = 0; j < PYRAMID_BAG_ITEMS_COUNT; ++j) {
         lu_BitstreamWrite_u8(state, v->quantity[i][j], 7);
      }
   }
}
