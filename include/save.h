#ifndef GUARD_SAVE_H
#define GUARD_SAVE_H

// Each 4 KiB flash sector contains 3968 bytes of actual data followed by a 128 byte footer.
// Only 12 bytes of the footer are used.
#define SECTOR_DATA_SIZE 3968
#define SECTOR_FOOTER_SIZE 128
#define SECTOR_SIZE (SECTOR_DATA_SIZE + SECTOR_FOOTER_SIZE)

#define NUM_SAVE_SLOTS 2

// If the sector's signature field is not this value then the sector is either invalid or empty.
#define SECTOR_SIGNATURE 0x8012025

#define SPECIAL_SECTOR_SENTINEL 0xB39D

//
// We need to distinguish between the different ways of identifying parts of a save file.
//
// Sector
//    A 4KiB slice of the 128KiB flash memory. There are 32 of these.
//
// Slot
//    The character, world, and Pokemon storage (PC) state for a save file. There are two 
//    slots, with one used as a backup save.
//
// Sector index
//    Sector indices span from 0 to 31 and describe a sector by its location.
//
// Sector ID
//    Sector IDs span from 0 to 31 and describe a sector by its purpose. Sectors within a 
//    slot have consistent IDs but are serialized to different locations, and so sector 
//    IDs don't always map 1:1 with sector indices. For example, sector ID 1 is part of 
//    the world state for a slot, but may be serialized to sector index 5.
//
//    Complicating matters is that slots 1 and 2 both use the same IDs for their sectors, 
//    but non-slot sectors may not even use meaningful IDs at all. A typical example (this 
//    one coming from a real save file) might look like this:
//
//          Index | ID | Note
//          ------+----+-------------------------
//          0     | 10 | Start of slot 1.
//          1     | 11 | 
//          2     | 12 | 
//          3     | 13 | 
//          4     | 0  | 
//          5     | 1  | 
//          6     | 2  | 
//          7     | 3  | 
//          8     | 4  | 
//          9     | 5  | 
//          10    | 6  | 
//          11    | 7  | 
//          12    | 8  | 
//          13    | 9  | 
//          14    | 11 | Start of slot 2.
//          15    | 12 | 
//          16    | 13 | 
//          17    | 0  | 
//          18    | 1  | 
//          19    | 2  | 
//          20    | 3  | 
//          21    | 4  | 
//          22    | 5  | 
//          23    | 6  | 
//          24    | 7  | 
//          25    | 8  | 
//          26    | 9  | 
//          27    | 10 | 
//          28    | -- | Non-slot sector: Hall of Fame (1)
//          29    | -- | Non-slot sector: Hall of Fame (2)
//          30    | -- | Non-slot sector: Trainer Hill
//          31    | -- | Non-slot sector: Recorded Battle
//

#define SECTOR_ID_SAVEBLOCK2          0
#define SECTOR_ID_SAVEBLOCK1_START    1
#define SECTOR_ID_SAVEBLOCK1_END      4
#define SECTOR_ID_PKMN_STORAGE_START  5
#define SECTOR_ID_PKMN_STORAGE_END   13
#define NUM_SECTORS_PER_SLOT         14
//
// Note: Both save slots' sectors use IDs 0 through 13 in their sector footers. The 
//       second save slot doesn't encode IDs as 14 through 27.
//
#define SECTOR_ID_HOF_1              28
#define SECTOR_ID_HOF_2              29
#define SECTOR_ID_TRAINER_HILL       30
#define SECTOR_ID_RECORDED_BATTLE    31
#define SECTORS_COUNT                32

#define NUM_HOF_SECTORS 2

#define SAVE_STATUS_EMPTY    0
#define SAVE_STATUS_OK       1
#define SAVE_STATUS_CORRUPT  2
#define SAVE_STATUS_NO_FLASH 4
#define SAVE_STATUS_ERROR    0xFF

// Special sector id value for certain save functions to
// indicate that no specific sector should be used.
#define FULL_SAVE_SLOT 0xFFFF

// SetDamagedSectorBits states
enum
{
    ENABLE,
    DISABLE,
    CHECK // unused
};

// Do save types
enum {
   SAVE_NORMAL,
    
   SAVE_LINK, // Link / Battle Frontier
   
   // Unused. Now a duplicate of SAVE_LINK.
   SAVE_EREADER,
    
   // Save: Used when replacing a pre-existing playthrough with a new one. Clears 
   // all non-slot sectors, and then overwrites the slot sectors.
   //
   // Load: pulls Hall of Fame data into `gDecompressionBuffer`, where the caller 
   // then reads it as needed. This includes loading the data, adding a new team, 
   // and then saving the modified data with a save call. 
   SAVE_HALL_OF_FAME,
    
   // Save: Used when replacing a pre-existing playthrough with a new one. Clears 
   // all non-slot sectors, and then overwrites the slot sectors.
   //
   // Load: Invalid. Not checked for.
   SAVE_OVERWRITE_DIFFERENT_FILE,
   
   // Unused. Code for it removed, to make savegame code easier to review.
   SAVE_HALL_OF_FAME_ERASE_BEFORE,
};

// A save sector location holds a pointer to the data for a particular sector
// and the size of that data. Size cannot be greater than SECTOR_DATA_SIZE.
struct SaveSectorLocation
{
    void *data;
    u16 size;
};

struct SaveSector
{
    u8 data[SECTOR_DATA_SIZE];
    u8 unused[SECTOR_FOOTER_SIZE - 12]; // Unused portion of the footer
    u16 id;
    u16 checksum;
    u32 signature; // must be SECTOR_SIGNATURE
    u32 counter;
}; // size is SECTOR_SIZE (0x1000)

#define SECTOR_SIGNATURE_OFFSET offsetof(struct SaveSector, signature)
#define SECTOR_COUNTER_OFFSET   offsetof(struct SaveSector, counter)

extern u16 gLastWrittenSector;
extern u32 gLastSaveCounter;
extern u16 gLastKnownGoodSector;
extern u32 gDamagedSaveSectors;
extern u32 gSaveCounter;
extern struct SaveSector *gFastSaveSector;
extern u16 gIncrementalSectorId;
extern u16 gSaveFileStatus;
extern void (*gGameContinueCallback)(void);
extern struct SaveSectorLocation gRamSaveSectorLocations[];

extern struct SaveSector gSaveDataBuffer;

void ClearSaveData(void);
void Save_ResetSaveCounters(void);
u8 HandleSavingData(u8 saveType);
u8 TrySavingData(u8 saveType);
bool8 LinkFullSave_Init(void);
bool8 LinkFullSave_WriteSector(void);
bool8 LinkFullSave_ReplaceLastSector(void);
bool8 LinkFullSave_SetLastSectorSignature(void);
bool8 WriteSaveBlock2(void);
bool8 WriteSaveBlock1Sector(void);
u8 LoadGameSave(u8 saveType);
u16 GetSaveBlocksPointersBaseOffset(void);
u32 TryReadSpecialSaveSector(u8 sector, u8 *dst);
u32 TryWriteSpecialSaveSector(u8 sector, u8 *src);
void Task_LinkFullSave(u8 taskId);

// save_failed_screen.c
void DoSaveFailedScreen(u8 saveType);

#endif // GUARD_SAVE_H
