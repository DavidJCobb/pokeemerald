#include "global.h"
#include "agb_flash.h"
#include "gba/flash_internal.h"
#include "fieldmap.h"
#include "save.h"
#include "task.h"
#include "decompress.h"
#include "load_save.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "main.h"
#include "trainer_hill.h"
#include "link.h"
#include "constants/game_stat.h"

#include "lu/generated/sector-serialize/CharacterData.h"
#include "lu/generated/sector-serialize/WorldData.h"
#include "lu/generated/sector-serialize/PokemonStorage.h"

#include "lu/generated/save-functors/save_functors.h"

struct SaveblockLayoutItem {
   u8 sliceNum;
   bool8(*func_read)(u8 sliceNum, const u8* src);
   bool8(*func_write)(u8 sliceNum, u8* dst);
};

static u16 CalculateChecksum(void*, u16);
static void ReadFlashSector(u8, struct SaveSector*);
static bool32 SetDamagedSectorBits(u8 op, u8 sectorId);
static u8 TryWriteSector(u8, u8*);
static u8 TryWriteSectorFooter(u8 sector_index, const struct SaveSector*);

static u8 GetSaveValidStatus();

static u8 TryLoadSaveSlot(const struct SaveblockLayoutItem*);
static u8 CopySaveSlotData(const struct SaveblockLayoutItem*);

static u8 ReadSectorWithSize(u8 sectorId, u8 *data, u16 size);

static u8 OnBeginFullSlotSave(bool8 isIncremental);
static u8 OnFailedFullSlotSave();

static u8 SerializeToSlotOwnedSector(u8 sectorId, bool8 eraseFlashFirst);
static u8 WriteSlot(bool8 cycleSectors, bool8 savePokemonStorage, bool8 eraseFlashFirst);
static u8 WriteSectorWithSize(u8 sectorId, u8* src, u16 size);
static void WriteHallOfFame();

// Divide save blocks into individual chunks to be written to flash sectors

/*
 * Sector Layout:
 *
 * Sectors 0 - 13:      Save Slot 1
 * Sectors 14 - 27:     Save Slot 2
 * Sectors 28 - 29:     Hall of Fame
 * Sector 30:           Trainer Hill
 * Sector 31:           Recorded Battle
 *
 * There are two save slots for saving the player's game data. We alternate between
 * them each time the game is saved, so that if the current save slot is corrupt,
 * we can load the previous one. We also rotate the sectors in each save slot
 * so that the same data is not always being written to the same sector. This
 * might be done to reduce wear on the flash memory, but I'm not sure, since all
 * 14 sectors get written anyway.
 *
 * See SECTOR_ID_* constants in save.h
 */

#define SAVEBLOCK_LAYOUT_ITEM(index, name)  {  index, &ReadSector_##name , &WriteSector_##name  }

static const struct SaveblockLayoutItem sSaveSlotLayout[NUM_SECTORS_PER_SLOT] = {
#include "lu/generated/save-functors/save_functor_table.inl"
};

u16 gLastWrittenSector;
u32 gLastSaveCounter;
u16 gLastKnownGoodSector;
u32 gDamagedSaveSectors;
u32 gSaveCounter;
struct SaveSector *gReadWriteSector; // Pointer to a buffer for reading/writing a sector
u16 gIncrementalSectorId;
u16 gSaveUnusedVar;
u16 gSaveFileStatus;
void (*gGameContinueCallback)(void);
struct SaveSectorLocation gRamSaveSectorLocations[NUM_SECTORS_PER_SLOT];
u16 gSaveUnusedVar2;
u16 gSaveAttemptStatus;

EWRAM_DATA struct SaveSector gSaveDataBuffer = {0}; // Buffer used for reading/writing sectors
EWRAM_DATA static u8 sUnusedVar = 0;


static u16 CalculateChecksum(void* data, u16 size) {
   u16 i;
   u32 checksum = 0;

   for (i = 0; i < (size / 4); i++) {
      checksum += *((u32*)data);
      data     += sizeof(u32);
   }

   return ((checksum >> 16) + checksum);
}
static void ReadFlashSector(u8 sectorId, struct SaveSector* dst) {
   ReadFlash(sectorId, 0, dst->data, SECTOR_SIZE);
}

static bool32 SetDamagedSectorBits(u8 op, u8 sectorId) {
   bool32 retVal = FALSE;

   switch (op) {
      case ENABLE:
         gDamagedSaveSectors |= (1 << sectorId);
         break;
      case DISABLE:
         gDamagedSaveSectors &= ~(1 << sectorId);
         break;
      case CHECK: // unused
         if (gDamagedSaveSectors & (1 << sectorId))
            retVal = TRUE;
         break;
   }

   return retVal;
}

static u8 TryWriteSector(u8 sector, u8 *data) {
   if (ProgramFlashSectorAndVerify(sector, data)) { // is damaged?
      // Failed
      SetDamagedSectorBits(ENABLE, sector);
      return SAVE_STATUS_ERROR;
   } else {
      // Succeeded
      SetDamagedSectorBits(DISABLE, sector);
      return SAVE_STATUS_OK;
   }
}
static u8 TryWriteSectorFooter(u8 sector_index, const struct SaveSector* sector_data) {
   u8  status;
   u16 i;
   
   for (i = 0; i < SECTOR_FOOTER_SIZE; i++) {
      if (ProgramFlashByte(sector_index, SECTOR_DATA_SIZE + i, ((u8*)sector_data)[SECTOR_DATA_SIZE + i])) {
         status = SAVE_STATUS_ERROR;
         break;
      }
   }

   if (status == SAVE_STATUS_ERROR) {
      // Writing signature/counter failed
      SetDamagedSectorBits(ENABLE, sector_index);
      return SAVE_STATUS_ERROR;
   }
   
   // Succeeded
   SetDamagedSectorBits(DISABLE, sector_index);
   return SAVE_STATUS_OK;
}


// This is not actually part of the save process. Rather, it reads directly into flash 
// memory and pulls out SaveBlock2 in order to read the trainer ID. This is fed into 
// Game Freak's ASLR code as part of the offset it shifts data by. The reason this has 
// to read into flash memory (i.e. the reason it exists at all) is because it's run 
// before you actually load your save file.
u16 GetSaveBlocksPointersBaseOffset(void) {
   u16 i, slotOffset;
   struct SaveSector* sector;

   sector = gReadWriteSector = &gSaveDataBuffer;
   if (gFlashMemoryPresent != TRUE)
      return 0;
   GetSaveValidStatus();
   slotOffset = NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);
   for (i = 0; i < NUM_SECTORS_PER_SLOT; i++) {
      ReadFlashSector(i + slotOffset, gReadWriteSector);

      // Base offset for SaveBlock2 is calculated using the trainer id
      if (gReadWriteSector->id == SECTOR_ID_SAVEBLOCK2)
         return sector->data[offsetof(struct SaveBlock2, playerTrainerId[0])] +
                sector->data[offsetof(struct SaveBlock2, playerTrainerId[1])] +
                sector->data[offsetof(struct SaveBlock2, playerTrainerId[2])] +
                sector->data[offsetof(struct SaveBlock2, playerTrainerId[3])];
   }
   return 0;
}




static u8 GetSaveValidStatus() {
   u8  slot_idx;
   u16 sector_idx;
   
   u8  slot_statuses[NUM_SAVE_SLOTS];
   u32 slot_counters[NUM_SAVE_SLOTS];
   
   for(slot_idx = 0; slot_idx < NUM_SAVE_SLOTS; ++slot_idx) {
      bool8 any_valid_signature = FALSE;
      u32   valid_sector_flags  = 0;
      
      for(sector_idx = 0; sector_idx < NUM_SECTORS_PER_SLOT; ++sector_idx) {
         ReadFlashSector(sector_idx + (slot_idx * NUM_SECTORS_PER_SLOT), gReadWriteSector);
         if (gReadWriteSector->signature == SECTOR_SIGNATURE) {
            u16 checksum;
            
            any_valid_signature = TRUE;
            checksum            = CalculateChecksum(gReadWriteSector->data, SECTOR_DATA_SIZE); // TODO: Could optimize by using only the bytespan of the serialized/bitpacked data.
            if (gReadWriteSector->checksum == checksum) {
               slot_counters[slot_idx] = gReadWriteSector->counter;
               valid_sector_flags |= 1 << gReadWriteSector->id;
            }
         }
      }
      if (any_valid_signature) {
         if (valid_sector_flags == (1 << NUM_SECTORS_PER_SLOT) - 1)
            slot_statuses[slot_idx] = SAVE_STATUS_OK;
         else
            slot_statuses[slot_idx] = SAVE_STATUS_ERROR;
      } else {
         // No sectors in this slot have the correct signature; treat it as empty.
         slot_statuses[slot_idx] = SAVE_STATUS_EMPTY;
      }
   }
   
   if (slot_statuses[0] == SAVE_STATUS_OK && slot_statuses[1] == SAVE_STATUS_OK) {
      if (
         (slot_counters[0] == -1 && slot_counters[1] ==  0)
      || (slot_counters[0] ==  0 && slot_counters[1] == -1)
      ) {
         if ((unsigned)(slot_counters[0] + 1) < (unsigned)(slot_counters[1] + 1))
            gSaveCounter = slot_counters[1];
         else
            gSaveCounter = slot_counters[0];
      } else {
         if (slot_counters[0] < slot_counters[1])
            gSaveCounter = slot_counters[1];
         else
            gSaveCounter = slot_counters[0];
      }
      return SAVE_STATUS_OK;
   }
   
   //
   // One or both save slots are not OK.
   //
   
   {
      u8    i;
      bool8 all_empty;
      
      all_empty = TRUE;
      for(i = 0; i < NUM_SAVE_SLOTS; ++i) {
         all_empty &= (slot_statuses[i] == SAVE_STATUS_EMPTY);
         
         if (slot_statuses[i] == SAVE_STATUS_OK) {
            gSaveCounter = slot_counters[i];
            
            if (slot_statuses[i ? 0 : 1] == SAVE_STATUS_ERROR)
               return SAVE_STATUS_ERROR; // Other slot errored.
            return SAVE_STATUS_OK; // This slot is OK; other slot is empty.
         }
      }
      
      if (all_empty) {
         gSaveCounter       = 0;
         gLastWrittenSector = 0;
         return SAVE_STATUS_EMPTY;
      }
   }

   // Both slots errored.
   gSaveCounter       = 0;
   gLastWrittenSector = 0;
   return SAVE_STATUS_CORRUPT;
}

u8 LoadGameSave(u8 saveType) {
   u8 status;

   if (gFlashMemoryPresent != TRUE) {
      DebugPrintf("Requested savegame load, but there's no flash memory!", 0);
      gSaveFileStatus = SAVE_STATUS_NO_FLASH;
      return SAVE_STATUS_ERROR;
   }

   switch (saveType) {
      case SAVE_NORMAL:
      default:
         DebugPrintf("Performing full savegame load...", 0);
         status = TryLoadSaveSlot(sSaveSlotLayout);
         CopyPartyAndObjectsFromSave();
         gSaveFileStatus = status;
         gGameContinueCallback = 0;
         DebugPrintf("Performed full savegame load.", 0);
         break;
      case SAVE_HALL_OF_FAME:
         status = ReadSectorWithSize(SECTOR_ID_HOF_1, gDecompressionBuffer, SECTOR_DATA_SIZE);
         if (status == SAVE_STATUS_OK) {
            status = ReadSectorWithSize(SECTOR_ID_HOF_2, &gDecompressionBuffer[SECTOR_DATA_SIZE], SECTOR_DATA_SIZE);
         }
         break;
   }

   return status;
}
static u8 TryLoadSaveSlot(const struct SaveblockLayoutItem* layout) {
   u8 status;
   
   gReadWriteSector = &gSaveDataBuffer;
   status = GetSaveValidStatus();
   CopySaveSlotData(layout);
   
   return status;
}
static u8 CopySaveSlotData(const struct SaveblockLayoutItem* layout) {
   u16 i;
   u16 checksum;
   u16 slotOffset = NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);
   u16 id;

   for (i = 0; i < NUM_SECTORS_PER_SLOT; i++) {
      ReadFlashSector(i + slotOffset, gReadWriteSector);

      id = gReadWriteSector->id;
      if (id == 0)
         gLastWrittenSector = i;

      checksum = CalculateChecksum(gReadWriteSector->data, SECTOR_DATA_SIZE); // TODO: Could optimize by using only the bytespan of the serialized/bitpacked data.

      // Only copy data for sectors whose signature and checksum fields are correct
      if (gReadWriteSector->signature == SECTOR_SIGNATURE && gReadWriteSector->checksum == checksum) {
         (layout[id].func_read)(layout[id].sliceNum, gReadWriteSector->data);
      }
   }

   return SAVE_STATUS_OK;
}

// Used for data not considered part of one of the normal save slots, i.e. the Hall of Fame, Trainer Hill, or Recorded Battle
static u8 ReadSectorWithSize(u8 sectorId, u8* dst, u16 size) {
   u16 i;
   
   struct SaveSector* sector = &gSaveDataBuffer;
   ReadFlashSector(sectorId, sector);
   if (sector->signature == SECTOR_SIGNATURE) {
      u16 checksum = CalculateChecksum(sector->data, size);
      if (sector->id == checksum) {
         // Signature and checksum are correct, copy data
         for (i = 0; i < size; i++)
            dst[i] = sector->data[i];
         return SAVE_STATUS_OK;
      } else {
         // Incorrect checksum
         return SAVE_STATUS_CORRUPT;
      }
   } else {
        // Incorrect signature value
        return SAVE_STATUS_EMPTY;
   }
}

// Trainer Hill or Recorded Battle
u32 TryReadSpecialSaveSector(u8 sector, u8 *dst) {
   s32 i;
   s32 size;
   u8* savData;

   if (sector != SECTOR_ID_TRAINER_HILL && sector != SECTOR_ID_RECORDED_BATTLE)
      return SAVE_STATUS_ERROR;

   ReadFlash(sector, 0, (u8*)&gSaveDataBuffer, SECTOR_SIZE);
   if (*(u32*)(&gSaveDataBuffer.data[0]) != SPECIAL_SECTOR_SENTINEL)
      return SAVE_STATUS_ERROR;

   // Copies whole save sector except u32 counter
   i       = 0;
   size    = SECTOR_COUNTER_OFFSET - 1;
   savData = &gSaveDataBuffer.data[4]; // data[4] to skip past SPECIAL_SECTOR_SENTINEL
   for (; i <= size; i++)
      dst[i] = savData[i];
   
   return SAVE_STATUS_OK;
}



//
// WRITING
//



static u8 OnBeginFullSlotSave(bool8 isIncremental) {
   gLastKnownGoodSector = gLastWrittenSector; // backup the current written sector before attempting to write.
   gLastSaveCounter     = gSaveCounter;
   gLastWrittenSector++;
   gLastWrittenSector = gLastWrittenSector % NUM_SECTORS_PER_SLOT;
   gSaveCounter++;
   if (isIncremental) {
      gIncrementalSectorId = 0;
   }
}
static u8 OnFailedFullSlotSave() {
   gLastWrittenSector = gLastKnownGoodSector;
   gSaveCounter       = gLastSaveCounter;
}

static u8 SerializeToSlotOwnedSector(u8 sectorId, bool8 eraseFlashFirst) {
   u16 sectorIndex;
   u8  status;
   //
   const struct SaveblockLayoutItem* sector_definition;
   
   sector_definition = &sSaveSlotLayout[sectorId];

   // Adjust sector id for current save slot
   sectorIndex = sectorId + gLastWrittenSector;
   sectorIndex %= NUM_SECTORS_PER_SLOT;
   sectorIndex += NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);

   // Clear temp save sector
   {
      u16 i;
      for (i = 0; i < SECTOR_SIZE; i++)
         ((u8*)gReadWriteSector)[i] = 0;
   }

   // Set footer data
   gReadWriteSector->id        = sectorId;
   gReadWriteSector->signature = SECTOR_SIGNATURE;
   gReadWriteSector->counter   = gSaveCounter;

   // Write current data to temp buffer for writing
   (sector_definition->func_write)(sector_definition->sliceNum, gReadWriteSector->data);

   gReadWriteSector->checksum = CalculateChecksum(gReadWriteSector->data, SECTOR_DATA_SIZE); // TODO: Could optimize by using only the bytespan of the serialized/bitpacked data.
   
   //
   // Commit the data to flash memory.
   //
   if (eraseFlashFirst) {
      EraseFlashSector(sectorIndex);
   }
   status = TryWriteSector(sectorIndex, gReadWriteSector->data);
   if (status == SAVE_STATUS_ERROR) {
      return status;
   }
   return TryWriteSectorFooter(sectorIndex, gReadWriteSector);
}


static u8 WriteSlot(bool8 cycleSectors, bool8 savePokemonStorage, bool8 eraseFlashFirst) {
   u8  i;
   u32 status;
   
   DebugPrintf("Performing full save...", 0);
   
   if (cycleSectors) {
      OnBeginFullSlotSave(FALSE);
   }
   status = SAVE_STATUS_OK;
   
   #if SECTOR_ID_SAVEBLOCK2 > 0
      #error Slot sector layout changed! Current code assumes [0, NUM_SECTORS_PER_SLOT) is all slotted.
   #endif
   #if (SECTOR_ID_SAVEBLOCK1_START   != SECTOR_ID_SAVEBLOCK2       + 1) \
    || (SECTOR_ID_PKMN_STORAGE_START != SECTOR_ID_SAVEBLOCK1_END   + 1) \
    || (NUM_SECTORS_PER_SLOT         != SECTOR_ID_PKMN_STORAGE_END + 1)
      #error Slot sector layout changed! Current code assumes layout is: SaveBlock2; SaveBlock1; PokemonStorage.
   #endif
   //
   if (savePokemonStorage) {
      // SaveBlock1, SaveBlock2, and PokemonStorage
      for (i = 0; i < NUM_SECTORS_PER_SLOT; i++)
         SerializeToSlotOwnedSector(i, eraseFlashFirst);
   } else {
      // SaveBlock1, SaveBlock2
      SerializeToSlotOwnedSector(SECTOR_ID_SAVEBLOCK2, eraseFlashFirst);
      for (i = SECTOR_ID_SAVEBLOCK1_START; i <= SECTOR_ID_SAVEBLOCK1_END; i++)
         SerializeToSlotOwnedSector(i, eraseFlashFirst);
   }
   
   DebugPrintf("Full save done (success or failure).", 0);
   
   if (gDamagedSaveSectors) {
      if (cycleSectors) {
         // Revert cycling.
         OnFailedFullSlotSave();
      }
      status = SAVE_STATUS_ERROR;
   }
   
   return status;
}
static u8 WriteSectorWithSize(u8 sectorIndex, u8* src, u16 size) { // should only be used for non-slot sectors
   u16 i;
   u8  status;
   struct SaveSector* sector = &gSaveDataBuffer;

   // Clear temp save sector
   for (i = 0; i < SECTOR_SIZE; i++)
      ((u8*)sector)[i] = 0;

   sector->signature = SECTOR_SIGNATURE;

   // Copy data to temp buffer for writing
   for (i = 0; i < size; i++)
      sector->data[i] = src[i];

   // though this appears to be incorrect, it might be some sector checksum instead of a whole save checksum and only appears to be relevent to HOF data, if used.
   sector->id = CalculateChecksum(src, size);
   
   status = TryWriteSector(sectorIndex, sector->data);
   if (status == SAVE_STATUS_ERROR) {
      return status;
   }
   return TryWriteSectorFooter(sectorIndex, sector);
}
static void WriteHallOfFame() {
   u8* tempAddr = gDecompressionBuffer;
   WriteSectorWithSize(SECTOR_ID_HOF_1, tempAddr,                    SECTOR_DATA_SIZE);
   WriteSectorWithSize(SECTOR_ID_HOF_2, tempAddr + SECTOR_DATA_SIZE, SECTOR_DATA_SIZE);
}


u8 TrySavingData(u8 saveType) {
   DebugPrintf("Received request to save... Type is %d.", saveType);
   if (gFlashMemoryPresent != TRUE) {
      gSaveAttemptStatus = SAVE_STATUS_ERROR;
      return SAVE_STATUS_ERROR;
   }

   HandleSavingData(saveType);
   if (!gDamagedSaveSectors) {
      DebugPrintf("Request to save was successful (hopefully)!", 0);
      gSaveAttemptStatus = SAVE_STATUS_OK;
      return SAVE_STATUS_OK;
   } else {
      DebugPrintf("Request to save failed!", 0);
      DoSaveFailedScreen(saveType);
      gSaveAttemptStatus = SAVE_STATUS_ERROR;
      return SAVE_STATUS_ERROR;
   }
}
u8 HandleSavingData(u8 saveType) {
   u8   i;
   u32* backupVar = gTrainerHillVBlankCounter;

   gTrainerHillVBlankCounter = NULL;
   
   switch (saveType) {
      case SAVE_HALL_OF_FAME:
         if (GetGameStat(GAME_STAT_ENTERED_HOF) < 999)
            IncrementGameStat(GAME_STAT_ENTERED_HOF);

         // Write the full save slot first
         CopyPartyAndObjectsToSave();
         WriteSlot(TRUE, TRUE, FALSE);

         // Save the Hall of Fame
         WriteHallOfFame();
         break;
         
      case SAVE_OVERWRITE_DIFFERENT_FILE:
         // Erase non-slot sectors (Hall of Fame; Trainer Hill; Recorded Battle)
         for (i = SECTOR_ID_HOF_1; i < SECTORS_COUNT; i++) {
            EraseFlashSector(i);
         }
         //
         // fallthrough
         //
      case SAVE_NORMAL:
      default:
         CopyPartyAndObjectsToSave();
         WriteSlot(TRUE, TRUE, FALSE);
         break;
         
      case SAVE_LINK:
      case SAVE_EREADER: // Dummied, now duplicate of SAVE_LINK
         // Used by link / Battle Frontier
         // Write only SaveBlocks 1 and 2 (skips the PC)
         CopyPartyAndObjectsToSave();
         WriteSlot(FALSE, FALSE, TRUE);
         break;
   }
   
   gTrainerHillVBlankCounter = backupVar;
   
   return 0;
}

// Trainer Hill or Recorded Battle
// This is almost the same as our `WriteSectorWithSize` save for the presence of a sentinel u32 placed 
// inside the sector before the data to be written.
u32 TryWriteSpecialSaveSector(u8 sector, u8* src) {
   s32   i;
   s32   size;
   u8*   savData;
   void* savDataBuffer;

   if (sector != SECTOR_ID_TRAINER_HILL && sector != SECTOR_ID_RECORDED_BATTLE)
      return SAVE_STATUS_ERROR;

   savDataBuffer = &gSaveDataBuffer;
   *(u32*)(savDataBuffer) = SPECIAL_SECTOR_SENTINEL;

   // Copies whole save sector except u32 counter
   i    = 0;
   size = SECTOR_COUNTER_OFFSET - 1;
   savData = &gSaveDataBuffer.data[4]; // data[4] to skip past SPECIAL_SECTOR_SENTINEL
   for (; i <= size; i++)
      savData[i] = src[i];
   
   if (ProgramFlashSectorAndVerify(sector, savDataBuffer) != 0)
      return SAVE_STATUS_ERROR;
   return SAVE_STATUS_OK;
}

// TODO: Rename to DoLinkIncrementalPartialSave_PartA
// Better yet: wrap both this and WriteSaveBlock1 in a function that writes the appropriate 
// sector based on gIncrementalSectorId.
u8 WriteSaveBlock2(void) {
   if (gFlashMemoryPresent != TRUE)
      return TRUE;

   CopyPartyAndObjectsToSave();
   {  // This is the same as `OnBeginFullSlotSave`, except we don't cycle to the next slot.
      gReadWriteSector     = &gSaveDataBuffer;
      gLastKnownGoodSector = gLastWrittenSector;
      gLastSaveCounter     = gSaveCounter;
      gIncrementalSectorId = 0;
      gDamagedSaveSectors  = 0;
   }
   
   SerializeToSlotOwnedSector(SECTOR_ID_SAVEBLOCK2, TRUE);
   if (gDamagedSaveSectors) {
      // This is the same as `OnFailedFullSlotSave`.
      gLastWrittenSector = gLastKnownGoodSector;
      gSaveCounter       = gLastSaveCounter;
   }
   
   gIncrementalSectorId = SECTOR_ID_SAVEBLOCK1_START;
   
   return FALSE;
}

// TODO: Rename to DoLinkIncrementalPartialSave_PartB
// Better yet: wrap both this and WriteSaveBlock1 in a function that writes the appropriate 
// sector based on gIncrementalSectorId.
//
// Used in conjunction with WriteSaveBlock2 to write both for certain link saves.
// This will be called repeatedly in a task, writing each sector of SaveBlock1 incrementally.
// It returns TRUE when finished.
bool8 WriteSaveBlock1Sector(void) {
   u8  finished = FALSE;
   u16 sectorId = ++gIncrementalSectorId; // Because WriteSaveBlock2 will have been called prior, this will be SECTOR_ID_SAVEBLOCK1_START
   if (sectorId <= SECTOR_ID_SAVEBLOCK1_END) {
      SerializeToSlotOwnedSector(sectorId, TRUE);
   } else {
      finished = TRUE;
   }

   if (gDamagedSaveSectors) {
      DoSaveFailedScreen(SAVE_LINK);
   }

   return finished;
}



#define tState         data[0]
#define tTimer         data[1]
#define tInBattleTower data[2]
//
// Note that this is very different from TrySavingData(SAVE_LINK).
// Most notably it does save the PC data.
//
void Task_LinkFullSave(u8 taskId) {
   s16 *data = gTasks[taskId].data;

   switch (tState) {
      case 0:
         gSoftResetDisabled = TRUE;
         tState = 1;
         break;
      case 1:
         SetLinkStandbyCallback();
         tState = 2;
         break;
      case 2:
         if (IsLinkTaskFinished()) {
            if (!tInBattleTower)
                SaveMapView();
            tState = 3;
         }
         break;
      case 3:
         if (!tInBattleTower)
            SetContinueGameWarpStatusToDynamicWarp();
         LinkFullSave_Init();
         tState = 4;
         break;
      case 4: // 5-frame wait.
         if (++tTimer == 5) {
            tTimer = 0;
            tState = 5;
         }
         break;
      case 5:
         if (LinkFullSave_WriteSector())
            tState = 6;
         else
            tState = 4; // Not finished, delay again
         break;
      case 6:
         LinkFullSave_ReplaceLastSector();
         tState = 7;
         break;
      case 7:
         if (!tInBattleTower)
            ClearContinueGameWarpStatus2();
         SetLinkStandbyCallback();
         tState = 8;
         break;
      case 8:
         if (IsLinkTaskFinished()) {
            LinkFullSave_SetLastSectorSignature();
            tState = 9;
         }
         break;
      case 9:
         SetLinkStandbyCallback();
         tState = 10;
         break;
      case 10:
         if (IsLinkTaskFinished())
            tState++;
         break;
      case 11:
         if (++tTimer > 5) {
            gSoftResetDisabled = FALSE;
            DestroyTask(taskId);
         }
         break;
   }
}
//
#undef tState
#undef tTimer
#undef tInBattleTower

// Code for incremental saves during a link.
//
// Not sure why this exists... Maybe to prevent the game from hanging during 
// a link save? Does the game do *that* much extra processing when linked?
//
// What's especially vile is that outside code has direct access to the save 
// process and its innards. There isn't just an "init" call and a "do next" 
// call; outside code calls into functions that manage sector signatures and 
// similar.
//
// The typical flow is:
//  - Init
//  - Write sectors in loop
//  - Replace last sector
//  - Make savegame edits
//  - Wait for link to end
//  - Set last sector signature
//
// In most cases, this is called exclusively through Task_LinkFullSave, but 
// there's one exception: trading, which has its own "link full save" task 
// that also calls these functions directly.

bool8 LinkFullSave_Init(void) {
   if (gFlashMemoryPresent != TRUE)
      return TRUE;
   CopyPartyAndObjectsToSave();
   OnBeginFullSlotSave(TRUE);
   return FALSE;
}
bool8 LinkFullSave_WriteSector(void) { // incremental save step; return TRUE on complete or error; FALSE if not yet done
   // TODO: Investigate renaming this to `LinkFullSave_DoNext`.
   u8 status;
   if (gIncrementalSectorId < NUM_SECTORS_PER_SLOT - 1) {
      status = SAVE_STATUS_OK;
      
      SerializeToSlotOwnedSector(gIncrementalSectorId, FALSE);
      gIncrementalSectorId++;
      if (gDamagedSaveSectors) {
         status = SAVE_STATUS_ERROR;
         OnFailedFullSlotSave();
      }
   } else {
      // Exceeded max sector, finished
      status = SAVE_STATUS_ERROR;
   }
   if (gDamagedSaveSectors)
      DoSaveFailedScreen(SAVE_NORMAL);

   // In this case "error" either means that an actual error was encountered
   // or that the given max sector has been reached (meaning it has finished successfully).
   // If there was an actual error the save failed screen above will also be shown.
   if (status == SAVE_STATUS_ERROR)
      return TRUE;
   else
      return FALSE;
}
bool8 LinkFullSave_ReplaceLastSector(void) {
   // TODO: Investigate renaming this to `LinkFullSave_Finalize`.
   // Alternatively, consider integrating this into the "do next" function, since the game is 
   // willing to pop a save-failed screen at any point in the process.
   
   //HandleReplaceSectorAndVerify(NUM_SECTORS_PER_SLOT, gRamSaveSectorLocations);
   //
   // I'm honestly not sure this was ever needed. I think what happened was that Game Freak's 
   // original `LinkFullSave_WriteSector` used `HandleWriteIncrementalSector`, which in turn 
   // used `HandleWriteSetor`, which did a full sector write but didn't call Game Freak's 
   // equivalent to our `OnFailedFullSlotSave`. Then, they used `HandleReplaceSectorAndVerify` 
   // to redundantly re-write the first sector, because that one *does* call Game Freak's 
   // equivalent to our `OnFailedFullSlotSave` at the end.
   
   if (gDamagedSaveSectors)
      DoSaveFailedScreen(SAVE_NORMAL);
   return FALSE;
}

bool8 LinkFullSave_SetLastSectorSignature(void) {
   // TODO: Investigate renaming this to `LinkFullSave_DoPostSaveBehaviors`.
   
   //CopySectorSignatureByte(NUM_SECTORS_PER_SLOT, gRamSaveSectorLocations);
   //
   // Should hopefully no longer be needed given our changes to the save process.
    
   if (gDamagedSaveSectors)
      DoSaveFailedScreen(SAVE_NORMAL);
   return FALSE;
}

// End of link incremental save code.






void ClearSaveData(void) {
   u16 i;

   // Clear the full save two sectors at a time
   for (i = 0; i < SECTORS_COUNT / 2; i++) {
      EraseFlashSector(i);
      EraseFlashSector(i + SECTORS_COUNT / 2);
   }
}
void Save_ResetSaveCounters(void) {
   gSaveCounter        = 0;
   gLastWrittenSector  = 0;
   gDamagedSaveSectors = 0;
}
