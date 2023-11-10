#include "lu/generated/struct-serialize//serialize_ObjectEvent.h"

#include "global.h"
#include "global.fieldmap.h" // struct definition

#include "lu/bitstreams.h"

// dependencies
#include "lu/generated/struct-serialize//serialize_Coords16.h"
#include "lu/generated/struct-serialize//serialize_Coords16.h"
#include "lu/generated/struct-serialize//serialize_Coords16.h"

void lu_BitstreamRead_ObjectEvent(struct lu_BitstreamState* state, struct ObjectEvent* v) {
   v->active = lu_BitstreamRead_bool(state);
   v->singleMovementActive = lu_BitstreamRead_bool(state);
   v->triggerGroundEffectsOnMove = lu_BitstreamRead_bool(state);
   v->triggerGroundEffectsOnStop = lu_BitstreamRead_bool(state);
   v->disableCoveringGroundEffects = lu_BitstreamRead_bool(state);
   v->landingJump = lu_BitstreamRead_bool(state);
   v->heldMovementActive = lu_BitstreamRead_bool(state);
   v->heldMovementFinished = lu_BitstreamRead_bool(state);
   v->frozen = lu_BitstreamRead_bool(state);
   v->facingDirectionLocked = lu_BitstreamRead_bool(state);
   v->disableAnim = lu_BitstreamRead_bool(state);
   v->enableAnim = lu_BitstreamRead_bool(state);
   v->inanimate = lu_BitstreamRead_bool(state);
   v->invisible = lu_BitstreamRead_bool(state);
   v->offScreen = lu_BitstreamRead_bool(state);
   v->trackedByCamera = lu_BitstreamRead_bool(state);
   v->isPlayer = lu_BitstreamRead_bool(state);
   v->hasReflection = lu_BitstreamRead_bool(state);
   v->inShortGrass = lu_BitstreamRead_bool(state);
   v->inShallowFlowingWater = lu_BitstreamRead_bool(state);
   v->inSandPile = lu_BitstreamRead_bool(state);
   v->inHotSprings = lu_BitstreamRead_bool(state);
   v->hasShadow = lu_BitstreamRead_bool(state);
   v->spriteAnimPausedBackup = lu_BitstreamRead_bool(state);
   v->spriteAffineAnimPausedBackup = lu_BitstreamRead_bool(state);
   v->disableJumpLandingGroundEffect = lu_BitstreamRead_bool(state);
   v->fixedPriority = lu_BitstreamRead_bool(state);
   v->hideReflection = lu_BitstreamRead_bool(state);
   v->spriteId = lu_BitstreamRead_u8(state, 8);
   v->graphicsId = lu_BitstreamRead_u8(state, 8);
   v->movementType = lu_BitstreamRead_u8(state, 8);
   v->trainerType = lu_BitstreamRead_u8(state, 8);
   v->localId = lu_BitstreamRead_u8(state, 8);
   v->mapNum = lu_BitstreamRead_u8(state, 8);
   v->mapGroup = lu_BitstreamRead_u8(state, 8);
   v->currentElevation = lu_BitstreamRead_u8(state, 4);
   v->previousElevation = lu_BitstreamRead_u8(state, 4);
   lu_BitstreamRead_Coords16(state, &v->initialCoords);
   lu_BitstreamRead_Coords16(state, &v->currentCoords);
   lu_BitstreamRead_Coords16(state, &v->previousCoords);
   v->facingDirection = lu_BitstreamRead_u8(state, 4);
   v->movementDirection = lu_BitstreamRead_u8(state, 4);
   v->rangeX = lu_BitstreamRead_u8(state, 4);
   v->rangeY = lu_BitstreamRead_u8(state, 4);
   v->fieldEffectSpriteId = lu_BitstreamRead_u8(state, 8);
   v->warpArrowSpriteId = lu_BitstreamRead_u8(state, 8);
   v->movementActionId = lu_BitstreamRead_u8(state, 8);
   v->trainerRange_berryTreeId = lu_BitstreamRead_u8(state, 8);
   v->currentMetatileBehavior = lu_BitstreamRead_u8(state, 8);
   v->previousMetatileBehavior = lu_BitstreamRead_u8(state, 8);
   v->previousMovementDirection = lu_BitstreamRead_u8(state, 8);
   v->directionSequenceIndex = lu_BitstreamRead_u8(state, 8);
   v->playerCopyableMovement = lu_BitstreamRead_u8(state, 8);
}

void lu_BitstreamWrite_ObjectEvent(struct lu_BitstreamState* state, const struct ObjectEvent* v) {
   lu_BitstreamWrite_bool(state, v->active);
   lu_BitstreamWrite_bool(state, v->singleMovementActive);
   lu_BitstreamWrite_bool(state, v->triggerGroundEffectsOnMove);
   lu_BitstreamWrite_bool(state, v->triggerGroundEffectsOnStop);
   lu_BitstreamWrite_bool(state, v->disableCoveringGroundEffects);
   lu_BitstreamWrite_bool(state, v->landingJump);
   lu_BitstreamWrite_bool(state, v->heldMovementActive);
   lu_BitstreamWrite_bool(state, v->heldMovementFinished);
   lu_BitstreamWrite_bool(state, v->frozen);
   lu_BitstreamWrite_bool(state, v->facingDirectionLocked);
   lu_BitstreamWrite_bool(state, v->disableAnim);
   lu_BitstreamWrite_bool(state, v->enableAnim);
   lu_BitstreamWrite_bool(state, v->inanimate);
   lu_BitstreamWrite_bool(state, v->invisible);
   lu_BitstreamWrite_bool(state, v->offScreen);
   lu_BitstreamWrite_bool(state, v->trackedByCamera);
   lu_BitstreamWrite_bool(state, v->isPlayer);
   lu_BitstreamWrite_bool(state, v->hasReflection);
   lu_BitstreamWrite_bool(state, v->inShortGrass);
   lu_BitstreamWrite_bool(state, v->inShallowFlowingWater);
   lu_BitstreamWrite_bool(state, v->inSandPile);
   lu_BitstreamWrite_bool(state, v->inHotSprings);
   lu_BitstreamWrite_bool(state, v->hasShadow);
   lu_BitstreamWrite_bool(state, v->spriteAnimPausedBackup);
   lu_BitstreamWrite_bool(state, v->spriteAffineAnimPausedBackup);
   lu_BitstreamWrite_bool(state, v->disableJumpLandingGroundEffect);
   lu_BitstreamWrite_bool(state, v->fixedPriority);
   lu_BitstreamWrite_bool(state, v->hideReflection);
   lu_BitstreamWrite_u8(state, v->spriteId, 8);
   lu_BitstreamWrite_u8(state, v->graphicsId, 8);
   lu_BitstreamWrite_u8(state, v->movementType, 8);
   lu_BitstreamWrite_u8(state, v->trainerType, 8);
   lu_BitstreamWrite_u8(state, v->localId, 8);
   lu_BitstreamWrite_u8(state, v->mapNum, 8);
   lu_BitstreamWrite_u8(state, v->mapGroup, 8);
   lu_BitstreamWrite_u8(state, v->currentElevation, 4);
   lu_BitstreamWrite_u8(state, v->previousElevation, 4);
   lu_BitstreamWrite_Coords16(state, &v->initialCoords);
   lu_BitstreamWrite_Coords16(state, &v->currentCoords);
   lu_BitstreamWrite_Coords16(state, &v->previousCoords);
   lu_BitstreamWrite_u8(state, v->facingDirection, 4);
   lu_BitstreamWrite_u8(state, v->movementDirection, 4);
   lu_BitstreamWrite_u8(state, v->rangeX, 4);
   lu_BitstreamWrite_u8(state, v->rangeY, 4);
   lu_BitstreamWrite_u8(state, v->fieldEffectSpriteId, 8);
   lu_BitstreamWrite_u8(state, v->warpArrowSpriteId, 8);
   lu_BitstreamWrite_u8(state, v->movementActionId, 8);
   lu_BitstreamWrite_u8(state, v->trainerRange_berryTreeId, 8);
   lu_BitstreamWrite_u8(state, v->currentMetatileBehavior, 8);
   lu_BitstreamWrite_u8(state, v->previousMetatileBehavior, 8);
   lu_BitstreamWrite_u8(state, v->previousMovementDirection, 8);
   lu_BitstreamWrite_u8(state, v->directionSequenceIndex, 8);
   lu_BitstreamWrite_u8(state, v->playerCopyableMovement, 8);
}
