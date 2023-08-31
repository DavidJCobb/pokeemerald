
# Savedata optimization research

I've written "bitstream" functions that can write bitfields into a buffer without having to rely on the compiler to align things properly, and such that data doesn't have to be put in a bitfield at run-time. I should use this to bitpack savedata more thoroughly.

## How saving works

The game stores savedata in the cartridge's 128KB flash memory. This memory is divided into 32 "sectors," each of them 4KB (0x1000 bytes) long including the header and footer, and `save.h` and `save.c` are organized such that:

* A sector may contain only one struct.
* A single struct may be spread across multiple sectors.

The `SaveSectorLocation` struct is used to keep track of what data in RAM we want to save for any given sector; the `gRamSaveSectorLocations` array is a list of these. In other words: this directly maps RAM locations to sectors. It's updated whenever ASLR on the saveblocks is updated.

### Anatomy of save sectors

Each save sector consists of 3968 (0xF80) usable bytes of data followed by a 128-byte footer. Most of the footer is actually unused, but it does contain some important data including a signature.

Sector mappings are listed in `save.c` and are as follows:

```text
     0 - 13 | Save Slot A   (14 sectors long)
    14 - 27 | Save Slot B   (14 sectors long)
    28 - 29 | Hall of Fame  ( 2 sectors long)
    30      | Trainer Hill
    31      | Recorded Battle
```

The game alternates between slots A and B as a failsafe, so that if a save file is corrupted, the previous one can be loaded. Sectors are also written in different orders with every safe, which the decomp team suspects was done to reduce wear on the flash memory. Notably, Hall of Fame data, Trainer Hill data, and your Recorded Battle are not duplicated; the save slot system is meant solely as a backup mechanism for a single playthrough, and not a way to host multiple playthroughs on a single cartridge.

Save data for a single save slot is split into two "save blocks," the former of which is nearly four sectors large -- and is split across four sectors; the save system can cope with a struct being too large to fit in a single sector. Additionally, the Pokemon Storage System takes 9 sectors.

```text
   SaveBlock2     |  1
   SaveBlock1     |  4
   PokemonStorage |  9
   ---------------+----
   Total          | 14
```

The overwhelming majority of gameplay and world state is stored in SaveBlock1. SaveBlock2 is used to hold "identity" information: the player's name and gender; their playtime; their options; their Pokedex; and som Battle Tower and link state. If we ever want to version savedata, it'd probably be best to write a single version number into SaveBlock2, though for things that we expect to revise more frequently than other data (e.g. custom game options) we could have a secondary version number just for those.

### Relevant funcs

* **HandleWriteSector**
  Given a sector ID and the list of `SaveSectorLocation`s, copy the data for the desired sector from RAM into a temporary buffer (`gReadWriteSector`). Calculate the checksum, and then call `TryWriteSector` to commit the buffer to a flash memory sector.
* **HandleWriteSectorNBytes**
  Same as `HandleWriteSector`, but uses arguments to specify the RAM location and the size to copy. Still only permits writing one RAM source to one sector. Used for Hall of Fame data, since that apparently has to be (de)compressed.
* **TryWriteSector**
  Takes a flash-side sector ID (i.e. a sector ID with cycling applied) and a buffer, and writes the buffer to the specified sector in flash memory.
* **HandleReplaceSector**
  Same as `HandleWriteSector`, but it clears the destination sector first. Used for link and Battle Frontier saves, for eReader saves, and by `WriteSaveBlock1Sector1 and `WriteSaveBlock2`; called both directly and indirectly (via `HandleReplaceSectorAndVerify`).

### Top-level flow for loading

* LoadGameSave
** UpdateSaveAddresses
** TryLoadSaveSlot *[normal]*
*** `gReadWriteSector = &gSaveDataBuffer`
*** CopySaveSlotData
**** ReadFlashSector
**** CalculateChecksum
**** *[Buffer copy, one byte at a time, from `gReadWriteSector` to the target RAM location specified in the `SaveSectorLocation`s.]*
** TryLoadSaveSector *[Hall of Fame]*
*** ReadFlashSector
*** CalculateChecksum
*** *[Buffer copy, one byte at a time, from `sector->data[i]` to the destination buffer passed as an argument.]*

### Top-level flow for saving

This is... messy. Multiple parts of the game engine perform game saves via the task system, each of which has their own flow.

* Berry blender (`LinkPlayAgainHandleSaving` in `berry_blender.c`)
** *[Except when noted, advances one state at a time with each invocation of the task callback.]*
** State 0
*** SetLinkStandbyCallback
** State 1
*** IsLinkTaskFinished()
*** Disable soft resets.
** State 2
*** WriteSaveBlock2
** State 3
*** Wait 10 frames
*** SetLinkStandbyCallback
** State 4
*** If `IsLinkTaskFinished()`, then run `WriteSaveBlock1Sector()` once per invocation. Once saveblock1 is fully saved, advance; else, rewind to state 3.
** State 5
*** No-op state.
** State 6
*** Wait 5 frames.
*** Enable soft resets.
*** Return `TRUE` (indicates completion).

In other words:

```pseudocode
SetLinkStandbyCallback; // basically has all players wait until synched, I think
await IsLinkTaskFinished();
disable soft resets;
WriteSaveBlock2;
do:
   wait 10 frames;
   await IsLinkTaskFinished();
while: WriteSaveBlock1Sector;
no-op;
enable soft resets;
```

* Link battle completion (`Task_SaveAfterLinkBattle` in `start_menu.c`)
** State 0
*** Graphics commands
*** If wireless and in union room:
**** If any partners are on FR/LG Japanese, advance to state 1; else, state 5.
*** Else, advance to state 1.
** State 1
*** SetContinueGameWarpStatusToDynamicWarp
*** WriteSaveBlock2
** State 2
*** Call `WriteSaveBlock1Sector` once per invocation until done; then, `ClearContinueGameWarpStatus2` and advance.
** State 3
*** Graphics command.
** State 4
*** Graphics commands and terminate current task.
** State 5
*** `CreateTask(Task_LinkFullSave, 5);`
** State 6
*** Once `Task_LinkFullSave` is no longer active, go to state 3.

In other words:

```pseudocode
bool did_link_full_save = false;

graphics;
if (wireless && union room) {
   if (!kanto_japanese) {
      did_link_full_save = true;
      await task Task_LinkFullSave, 5; // states 5 and 6
         // disable soft resets
         // SetLinkStandbyCallback
         // await IsLinkTaskFinished
         // SaveMapView, if not in battle tower
         // SetContinueGameWarpStatusToDynamicWarp, if not in battle tower
         // LinkFullSave_Init
            // UpdateSaveAddresses
            // CopyPartyAndObjectsToSave
               // reminder: These are stored in SaveBlock1.
            // RestoreSaveBackupVarsAndIncrement
               // forces gIncrementalSectorId to 0
         // 5-frame delay
         // repeat LinkFullSave_WriteSector until finished or until error, with a 5-frame delay between sectors
            /* The effect of this repeated call is to save the full save slot. */
            // HandleWriteIncrementalSector(NUM_SECTORS_PER_SLOT, gRamSaveSectorLocations);
               // HandleWriteSector
            // conditionally show "save failed" screen if any save sectors damaged
         // LinkFullSave_ReplaceLastSector
            // HandleReplaceSectorAndVerify(NUM_SECTORS_PER_SLOT, gRamSaveSectorLocations);
               // subtracts one from the first argument to get a sector ID; this works out to SECTOR_ID_PKMN_STORAGE_END
               // HandleReplaceSector
            // conditionally show "save failed" screen if any save sectors damaged
         // ClearContinueGameWarpStatus2, if not in battle tower
         // await IsLinkTaskFinished
         // LinkFullSave_SetLastSectorSignature
            // CopySectorSignatureByte(NUM_SECTORS_PER_SLOT, gRamSaveSectorLocations);
            // conditionally show "save failed" screen if any save sectors damaged
         // SetLinkStandbyCallback
         // await IsLinkTaskFinished
         // enable soft resets
   }
} else {
   disable soft resets;
}
if (!did_link_full_save) {
   SetContinueGameWarpStatusToDynamicWarp;
   WriteSaveBlock2;
      // UpdateSaveAddresses
      // CopyPartyAndObjectsToSave
      // RestoreSaveBackupVars
         // forces gIncrementalSectorId to 0, so gIncrementalSectorId + 1 == SECTOR_ID_SAVEBLOCK2.
      // HandleReplaceSectorAndVerify(gIncrementalSectorId + 1, gRamSaveSectorLocations);
   for i from 0 through 3:
      WriteSaveBlock1Sector;
   ClearContinueGameWarpStatus2;
   enable soft resets;
}
graphics; // state 3
graphics; // state 4
exit;     // state 4
```

* Record mixing (`Task_DoRecordMixing` in `record_mixing.c`)
** *[Except when noted, advances one state at a time with each invocation of the task callback.]*
** State 0
*** No-op state.
** State 1
*** If any players are on Ruby or Sapphire, advance to next state; else, skip to state 6.
** State 2
*** SetContinueGameWarpStatusToDynamicWarp
*** WriteSaveBlock2
** State 3
*** Call `WriteSaveBlock1Sector` once per invocation until done; then, `ClearContinueGameWarpStatus2` and advance.
** State 4
*** Wait 10 frames; then call `SetCloseLinkCallback` and advance
** State 5
*** End of Ruby/Saphpire record mixing; `DestroyTask`.
** State 6
*** `Rfu_SetLinkRecovery(FALSE)`
*** `CreateTask(Task_LinkFullSave, 5)`
** State 7
*** *[Wait for `Task_LinkFullSave` to finish.]*
*** If wireless, call `Rfu_SetLinkRecovery(TRUE)` and advance to state 8. Else, move to state 4.
** State 8
*** SetLinkStandbyCallback
** State 9
*** When `IsLinkTaskFinished`, destroy this task.

In other words:

```pseudocode
if (any_ruby_or_sapphire) {
   SetContinueGameWarpStatusToDynamicWarp;
   WriteSaveBlock2;
   for i from 0 through 3:
      WriteSaveBlock1Sector;
   ClearContinueGameWarpStatus2;
} else {
   Rfu_SetLinkRecovery(FALSE);
   await task Task_LinkFullSave, 5;
      // disable soft resets
      // SetLinkStandbyCallback
      // await IsLinkTaskFinished
      // SaveMapView, if not in battle tower
      // SetContinueGameWarpStatusToDynamicWarp, if not in battle tower
      // LinkFullSave_Init
      // LinkFullSave_WriteSector
         // HandleWriteIncrementalSector, for the whole save slot
      // LinkFullSave_ReplaceLastSector
      // ClearContinueGameWarpStatus2, if not in battle tower
      // await IsLinkTaskFinished
      // LinkFullSave_SetLastSectorSignature
      // SetLinkStandbyCallback
      // await IsLinkTaskFinished
      // enable soft resets
   if (wireless) {
      Rfu_SetLinkRecovery(TRUE);
      SetLinkStandbyCallback;
      await IsLinkTaskFinished();
      exit;
   }
}
wait 10 frames;
SetCloseLinkCallback;
```

* `Task_LinkFullSave` (in `save.c`)
** *// this doesn't *just* save; it also performs link communication/synchronization of some sort.*
** State 0
*** Disable soft resets.
** State 1
*** SetLinkStandbyCallback
** State 2
*** Once `IsLinkTaskFinished()`, call `SaveMapView()` if not in the Battle Tower, and then advance.
** State 3
*** If not in the battle tower, `SetContinueGameWarpStatusToDynamicWarp()`.
*** LinkFullSave_Init
** State 4
*** Wait 5 frames.
** State 5
*** If `LinkFullSave_WriteSector()` succeeds, advance; else, rewind to state 4 (i.e. wait and try again).
** State 6
*** LinkFullSave_ReplaceLastSector
** State 7
*** If not in the battle tower, `ClearContinueGameWarpStatus2`.
*** SetLinkStandbyCallback
** State 8
*** Wait until `IsLinkTaskFinished()`.
*** LinkFullSave_SetLastSectorSignature
** State 9
*** SetLinkStandbyCallback
** State 10
*** Wait until `IsLinkTaskFinished()`
** State 11
*** Enable soft resets.
*** Terminate this task.

* `TrySavingData(u8 saveType)` (in `save.c`)
** *[used by...]*
*** `battle_pike.c`
*** `battle_pyramid.c`
*** `contest_util.c`
*** `frontier_util.c`
*** `hall_of_fame.c`
*** `link.c`
*** `mystery_event_menu.c`
*** `mystery_gift_menu.c
*** `start_menu.c` (all normal pause-menu saves)
*** `union_room_chat.c`
** If no flash memory is present, fail with an error.
** `HandleSavingData(saveType)`
*** Switch on the save type:
**** `SAVE_HALL_OF_FAME_ERASE_BEFORE`
***** EraseFlashSector in a loop
***** Fallthrough...
**** `SAVE_HALL_OF_FAME`
***** `IncrementGameStat(GAME_STAT_ENTERED_HOF);` unless at maximum (999)
***** CopyPartyAndObjectsToSave
***** `WriteSaveSectorOrSlot(FULL_SAVE_SLOT, gRamSaveSectorLocations);`
***** Use `HandleWriteSectorNBytes` to save Hall of Fame.
**** `SAVE_NORMAL`
***** CopyPartyAndObjectsToSave
***** `WriteSaveSectorOrSlot(FULL_SAVE_SLOT, gRamSaveSectorLocations);`
**** `SAVE_LINK` or `SAVE_EREADER`
***** *// used for link battles, Battle Frontier, or eReader*
***** CopyPartyAndObjectsToSave
***** Use `HandleReplaceSector` to save Saveblocks 2 and 1
***** Use `WriteSectorSignatureByte_NoOffset` to fix up incomplete data written by `HandleReplaceSector`
**** `SAVE_OVERWRITE_DIFFERENT_FILE`
***** Erase Hall of Fame using `EraseFlashSector`
***** CopyPartyAndObjectsToSave
***** `WriteSaveSectorOrSlot(FULL_SAVE_SLOT, gRamSaveSectorLocations);`
** Return a status code based on whether damaged save sectors exist.

## Objectives

The ultimate goals are:

* Bitpack SaveBlock1.
* Bitpack SaveBlock2.
* Bitpack, save, and load CustomGameOptions.

### Gaining control over what's in each sector

There are multiple save routines &mdash; several with link-specific communication done throughout the save process. Unless and until I fully decode the game's link systems (cable and wireless both), figuring out how to merge and harmonize everything into a single save process is not viable. Instead, we should aim a level lower, and rewrite how RAM data is mapped to save sectors. Everything that *needs* to save will pass through this mapping, no matter what the higher-level process is.

Currently, every sector in the save file maps to a single region of RAM. This mapping takes the form of `gRamSaveSectorLocations`, an array of `SaveSectorLocation` structs each containing a base pointer (to RAM) and size; the game just does a blind buffer copy from RAM to flash memory for each one. Since we plan on bitpacking the data, this means that to make the *minimum possible changes, regardless of downsides*, we'd need to:

* Reserve a RAM location for bitpacked savedata.
* Prior to saving, bitpack savedata from its normal location to the secondary RAM location.
* Remap the save sector locations to our bitpacked savedata.

However, that's a complete waste of RAM. A *more robust but more effortful solution* would be to:

* Instead of storing an array of `SaveSectorLocation`s, create a `SaveSectorHandlers` struct containing two function pointers: one to serialize data to a sector, and one to parse it from a sector. These would be invoked with two arguments: a pointer to the destination; and the sector ID (e.g. SECTOR_ID_PKMN_STORAGE_START).
* Write the following functions:
** SerializeSector_Read_SaveBlock1AndAddenda
*** *// would be listed four times*
*** *// uses static variables to track state, as per below*
** SerializeSector_Read_SaveBlock2AndAddenda
** SerializeSector_Read_PokemonStorageSystem
*** *// this would be a blind buffer copy as in vanilla, for now at least*
*** *// use the sector ID argument to know what slice of the struct to copy*
** SerializeSector_Read_HallOfFame1
*** *// this would be whatever vanilla does; apparently they compress HoF data somehow?*
** SerializeSector_Read_HallOfFame2
*** *// this would be whatever vanilla does; apparently they compress HoF data somehow?*
** SerializeSector_Write_*Whatever*

### Bitpacking data

If we're working in pure C, then the most maintainable way to handle bitpacking and serializing would be to define an array of structs, each consisting of:

* A pointer-to-member in SaveBlock1 or SaveBlock2
** (Actually, C doesn't have those; we'd use `offsetof`.)
* A typecode, e.g. u8 or u16, for the pointed-to value
* The desired bitcount to use

When saving, we'd just iterate over that array and write values in, tracking how many bits we've written into the current sector as we go. Once we see that the next value would overflow the sector, we stop; and then for the next SaveBlock1 write, we continue from where we left off. We lose compile-time verification that we're staying within savedata size limits, but we could potentially...

* ...add a function that steps through the list and just counts the sizes up.
* ...add a compile-time flag that calls that function on startup (i.e. at the title screen).
* ...display a debug error message on-screen and lock the game completely if we go over our budget.

The problem with this whole idea is that some fields in SaveBlock1, SaveBlock2, and their various constituent substructures are *already* bitpacked i.e. they're C bitfields, and you can't take the address of a C bitfield member. This means that pointer-to-members can't exist for those fields. I don't want to prevent the use of bitfields for flags and other non-essential data in memory, so we'll probably need to move the bitfields into a union (so we can read them, or read their containing u8 or u16), and then extend the struct above with the ability to access the Nth bits in a pointed-to bitmask starting from bit offset X within that mask. C90 (the weird C version GCC uses which is between C89 and C99) doesn't support anonymous unions, so this'll be... painful, akin to renaming every such field and having to update all references to them.

#### Wait, what about substructures and arrays?

We'll need to examine the fully expanded struct definitions for SaveBlock1 and SaveBlock2, and figure out some way to cope with substructures and arrays. We could use codegen (or copying and pasting @_@) to "flatten" the entire struct definitions, arrays included, into a single flat list of bitfields; but that feels ridiculously messy.

Possibly, we could just... make the above lists nestable and subscriptable. Like, have a list of bitfields for, say, `DaycareMon`, and then be able to have the daycare data's bitfield list say, "DaycareMon bitfield list x 5." If we need to interrupt serialization at a sector boundary, we'd just store as state: a pointer to the current bitfield list; a pointer to the current item in that list; and a fixed-length array of indices representing how far down we are (e.g. `foo[3].bar[5].baz[1]` -> `3, 5, 1`).

This is the model we'd want to use, then:

```c
enum {
   BITFIELD_TYPE_BOOL,
   BITFIELD_TYPE_U8,
   BITFIELD_TYPE_U16,
   BITFIELD_TYPE_U32,
   BITFIELD_TYPE_BUFFER,
   BITFIELD_TYPE_STRING,
};

struct Bitfield {
   #if _DEBUG // or whatever the GCC equivalent is
      const u8* name;
   #endif
   
   // Offset within the RAM-side struct.
   u16 offset;
   struct {
      //
      // These fields handle the case of a Bitfield being, well, a bitfield 
      // even in RAM.
      //
      u8 bit_count  = 0; // 0 == not a bitfield in RAM
      u8 bit_offset = 0;
   } src_bitfield_info;
   
   // Type enum; used to inform how we serialize certain values.
   u8 type;
   union {
      //
      // Neither of these values are used when the type is BITFIELD_TYPE_BOOL.
      //
      u8 bits;  // for everything except bools, buffers, and strings
      u8 chars; // for buffers and strings
   } size;
   //
   // For strings, we encode the number of chars as a prefix rather than 
   // encoding an EOS string terminator. However, if the string uses fewer 
   // than the chars available, we still encode all chars, with unused chars 
   // as EOS. This is because the savedata must always be able to hold the 
   // largest possible data; variable-length data is a liability here, not 
   // an asset.
   //
   // So for example, given a 7-character player name, we only need 3 bits 
   // to store the name length:
   //
   //    "Lucy"    -> 0b100 'L' 'u' 'c' 'y' EOS EOS EOS
   //    "Annabel" -> 0b111 'A' 'n' 'n' 'a' 'b' 'e' 'l'
   //
   // This is cheaper, in terms of bitcount, than encoding an EOS terminator 
   // without a length prefix. (In fact, we want to expand name lengths to 
   // 12 characters if possible, right? Well, this scheme saves us 4 bits for 
   // every player name in the save data -- a total of 277 bytes!)
   //
   // TODO: For BoxPokemon, do they pad shorter player names with EOS or with 
   // 0x00? We'll need to get that consistent to ensure checksums don't break 
   // after save/load (I assume). We'll probably want to double-check any 
   // savegame strings that get checksummed (directly or indirectly), actually.
   //
   // For that matter: we plan on forcing values back into range when saving, 
   // e.g. an item quantity of 100 gets forced to 99, just because that's the 
   // only real way to account for truncation. We may want to force values into 
   // range first; then recalc any checksums on the savedata; then save... or 
   // just not serialize checksums at all except for on Pokemon, apply one big 
   // checksum to the whole serialized stream, and then recalc checksums on load.
};

struct BitfieldListItem {
   struct BitfieldList* parent;
   union {
      struct Bitfield      field;
      struct BitfieldList* list;
   } data;
   bool8 is_nested_list : 1;
   u8    array_count    : 7;
};

struct BitfieldList {
   #if _DEBUG // or whatever the GCC equivalent is
      const u8* name;
   #endif
   u16 field_count;
   struct BitfieldListItem fields[];
};

struct PausedSerializationState {
   struct BitfieldList*     top_level_list;
   struct BitfieldListItem* next;
   u8 array_indices[8];
};
// We need a top-level list pointer because when we're traversing up out of a 
// BitfieldListItem, the top-level list has no "parent" pointer; we can't just 
// do `if (parent == NULL)`. Instead, when we want to back out to a parent, we 
// have to check if the list we're backing out of -- the list we've finished 
// serializing -- is the top-level list, and if so, then we're done serializing.
```

And a quick-and-dirty example of some bitfield definitions:

```c
void _set_list_item_parent_pointers(struct BitfieldList*);

#if _DEBUG // or whatever the GCC equivalent is
   #define BITFIELD_NAME(a_name) .name = #a_name,
#else
   #define BITFIELD_NAME(a_name)
#endif

#define BITFIELD_INFO(s, a_name) BITFIELD_NAME(a_name) .offset = offsetof(s, a_name)
#define BITFIELD_INFO_SRCBITS(s, a_name, a_cnt) BITFIELD_NAME(a_name) .offset = offsetof(s, a_name), .src_bitfield = a_cnt
#define BITCOUNT(n) .size = { .bits = n }
#define CHARCOUNT(n) .size = { .chars = n }

#define BITFIELD_LIST_NAME(struct_name) fields_##struct_name

#define _BITFIELD(struct_name, field_name, field_type, dst_bitcount) \
   { \
      .parent = &BITFIELD_LIST_NAME(struct_name), \
      .data = { \
         .field = { \
            BITFIELD_INFO(struct_name, field_name), \
            .type = field_type, \
            BITCOUNT(dst_bitcount), \
         } \
      }, \
      .is_nested_list = FALSE, \
      .array_count    = 0, \
   },
#define _BITSTRING(struct_name, field_name, charcount) \
   { \
      .parent = &BITFIELD_LIST_NAME(struct_name), \
      .data = { \
         .field = { \
            BITFIELD_INFO(struct_name, field_name), \
            .type = BITFIELD_TYPE_STRING, \
            CHARCOUNT(charcount), \
         } \
      }, \
      .is_nested_list = FALSE, \
      .array_count    = 0, \
   },
#define _BITBUFFER(struct_name, field_name, src_bytecount) \
   { \
      .parent = &BITFIELD_LIST_NAME(struct_name), \
      .data = { \
         .field = { \
            BITFIELD_INFO(struct_name, field_name), \
            .type = BITFIELD_TYPE_BUFFER, \
            CHARCOUNT(src_bytecount), \
         } \
      }, \
      .is_nested_list = FALSE, \
      .array_count    = 0, \
   },
   
#define BITFIELD_U8(struct_name, field_name, dst_bitcount) _BITFIELD(struct_name, field_name, BITFIELD_TYPE_U8, dst_bitcount)
#define BITFIELD_U16(struct_name, field_name, dst_bitcount) _BITFIELD(struct_name, field_name, BITFIELD_TYPE_U16, dst_bitcount)
#define BITFIELD_U32(struct_name, field_name, dst_bitcount) _BITFIELD(struct_name, field_name, BITFIELD_TYPE_U32, dst_bitcount)
#define BITFIELD_BUFFER(struct_name, field_name, src_bytecount) _BITBUFFER(struct_name, field_name, src_bytecount)
#define BITFIELD_STRING(struct_name, field_name, charcount) _BITSTRING(struct_name, field_name, charcount)

#define BITFIELD_TO_BITFIELD(struct_name, field_name, surrogate_struct_name, surrogate_field_name, surrogate_field_type, src_bitcount, src_bit_offset, dst_bitcount) \
   { \
      .parent = &BITFIELD_LIST_NAME(struct_name), \
      .data = { \
         .field = { \
            BITFIELD_NAME(field_name), \
            .type = surrogate_field_type, \
            .src_bitfield_info = { \
               .bit_count  = src_bitcount, \
               .bit_offset = src_bit_offset, \
            }, \
            BITCOUNT(dst_bitcount), \
         } \
      }, \
      .is_nested_list = FALSE, \
      .array_count    = 0, \
   },

// bitfields are not addressable, so we'll need pointer casts
struct _BoxPokemon_Surrogate {
   u32 personality;
   u32 otId;
   u8 nickname[POKEMON_NAME_LENGTH];
   u8 language;
   u8 bitfields_a;
      // isBadEgg   : 1
      // hasSpecies : 1
      // isEgg      : 1
      // unused     : 5
   u8 otName[PLAYER_NAME_LENGTH];
   u8 markings;
   u16 checksum;
   u16 unknown;
   union {
      u32 raw[(NUM_SUBSTRUCT_BYTES * 4) / 4]; // *4 because there are 4 substructs, /4 because it's u32, not u8
        union PokemonSubstruct substructs[4];
   } secure;
};

extern struct BitfieldList BITFIELD_LIST_NAME(BoxPokemon);
extern struct BitfieldList BITFIELD_LIST_NAME(BoxPokemon) = {
   BITFIELD_NAME("BoxPokemon")
   .field_count = 16,
   .fields = {
      BITFIELD_U32(BoxPokemon, personality, 32),
      BITFIELD_U32(BoxPokemon, otId, 32),
      BITFIELD_STRING(BoxPokemon, nickname, POKEMON_NAME_LENGTH),
      BITFIELD_U8(BoxPokemon, language, 8),
      BITFIELD_TO_BITFIELD(
         BoxPokemon,            isBadEgg,
         _BoxPokemon_Surrogate, bitfields_a,
         BITFIELD_TYPE_U8,
         1, // src bitfield bitcount
         0, // src bitfield offset
         1  // dst bitfield bitcount
      ),
      BITFIELD_TO_BITFIELD(
         BoxPokemon,            hasSpecies,
         _BoxPokemon_Surrogate, bitfields_a,
         BITFIELD_TYPE_U8,
         1, // src bitfield bitcount
         1, // src bitfield offset
         1  // dst bitfield bitcount
      ),
      BITFIELD_TO_BITFIELD(
         BoxPokemon,            isEgg,
         _BoxPokemon_Surrogate, bitfields_a,
         BITFIELD_TYPE_U8,
         1, // src bitfield bitcount
         2, // src bitfield offset
         1  // dst bitfield bitcount
      ),
      BITFIELD_TO_BITFIELD(
         BoxPokemon,            unused,     // Can we skip serializing this, or will that break the checksum?
         _BoxPokemon_Surrogate, bitfields_a,
         BITFIELD_TYPE_U8,
         5, // src bitfield bitcount
         3, // src bitfield offset
         5  // dst bitfield bitcount
      ),
      BITFIELD_STRING(BoxPokemon, otName, PLAYER_NAME_LENGTH),
      BITFIELD_U8(BoxPokemon, markings, 8),
      BITFIELD_U16(BoxPokemon, checksum, 16),
      BITFIELD_BUFFER(BoxPokemon, secure.raw, NUM_SUBSTRUCT_BYTES * 4),
   }
};
```

No, no, no, this isn't viable.


#### The problems with doing bitpack code entirely in the game

The above isn't maintainable. It's basically metaprogramming but with macros, and hand-rolling the things that a language with reflection would give us. We'd only be able to test it in-game, since the entire system is intrinsically linked to savedata, and we'd need a huge variety of end-game and Battle Frontier savestates to test it with -- savestates that we'd have to create ourselves, while making no other revisions to the game that might change memory layouts. That isn't even remotely viable.

So let's do code generation instead.

Suppose we build something that can scan C headers and pull out all struct definitions. C has a vastly simpler syntax than C++, so this is mostly viable. We pull the struct definitions and generate bitfield listings for them -- same as above, but in an actual OO language -- and then we start at SaveBlock1 and SaveBlock2, and generate serialization code. We get all the benefits of perfectly hand-rolled code, where we pause serialization at a sector boundary and then, for the next sector, resume from exactly the place we left off at; but we get these benefits automatically, without having to manually track offsets and sizes. We don't need run-time verification, and we don't need the overhead of run-time state-keeping; the game can serialize "blindly."

Better still: this is something we could test "in a lab." We could throw test structs at this program, vary the sector sizes used for tests, and more. I'd probably prototype it in JavaScript first but if we eventually move it to C++, then we can invoke it via the command line (maybe integrate it into the existing build system if we're feeling adventurous?) and statically assert that it produces correct results under a variety of conditions.
