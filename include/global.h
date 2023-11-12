#ifndef GUARD_GLOBAL_H
#define GUARD_GLOBAL_H

#include <string.h>
#include <limits.h>
#include "config.h" // we need to define config before gba headers as print stuff needs the functions nulled before defines.
#include "gba/gba.h"
#include "constants/global.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/species.h"
#include "constants/pokedex.h"
#include "constants/berry.h"
#include "constants/maps.h"
#include "constants/pokemon.h"
#include "constants/easy_chat.h"
#include "constants/trainer_hill.h"

// Prevent cross-jump optimization.
#define BLOCK_CROSS_JUMP asm("");

// to help in decompiling
#define asm_unified(x) asm(".syntax unified\n" x "\n.syntax divided")
#define NAKED __attribute__((naked))

/// IDE support
#if defined(__APPLE__) || defined(__CYGWIN__) || defined(__INTELLISENSE__)
// We define these when using certain IDEs to fool preproc
#define _(x)        {x}
#define __(x)       {x}
#define INCBIN(...) {0}
#define INCBIN_U8   INCBIN
#define INCBIN_U16  INCBIN
#define INCBIN_U32  INCBIN
#define INCBIN_S8   INCBIN
#define INCBIN_S16  INCBIN
#define INCBIN_S32  INCBIN
#endif // IDE support

#define ARRAY_COUNT(array) (size_t)(sizeof(array) / sizeof((array)[0]))

// GameFreak used a macro called "NELEMS", as evidenced by
// AgbAssert calls.
#define NELEMS(arr) (sizeof(arr)/sizeof(*(arr)))

#define SWAP(a, b, temp)    \
{                           \
    temp = a;               \
    a = b;                  \
    b = temp;               \
}

// useful math macros

// Converts a number to Q8.8 fixed-point format
#define Q_8_8(n) ((s16)((n) * 256))

// Converts a number to Q4.12 fixed-point format
#define Q_4_12(n)  ((s16)((n) * 4096))

// Converts a number to Q24.8 fixed-point format
#define Q_24_8(n)  ((s32)((n) << 8))

// Converts a Q8.8 fixed-point format number to a regular integer
#define Q_8_8_TO_INT(n) ((int)((n) / 256))

// Converts a Q4.12 fixed-point format number to a regular integer
#define Q_4_12_TO_INT(n)  ((int)((n) / 4096))

// Converts a Q24.8 fixed-point format number to a regular integer
#define Q_24_8_TO_INT(n) ((int)((n) >> 8))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

#if MODERN
#define abs(x) (((x) < 0) ? -(x) : (x))
#endif

// Used in cases where division by 0 can occur in the retail version.
// Avoids invalid opcodes on some emulators, and the otherwise UB.
#ifdef UBFIX
#define SAFE_DIV(a, b) ((b) ? (a) / (b) : 0)
#else
#define SAFE_DIV(a, b) ((a) / (b))
#endif

// Extracts the upper 16 bits of a 32-bit number
#define HIHALF(n) (((n) & 0xFFFF0000) >> 16)

// Extracts the lower 16 bits of a 32-bit number
#define LOHALF(n) ((n) & 0xFFFF)

// There are many quirks in the source code which have overarching behavioral differences from
// a number of other files. For example, diploma.c seems to declare rodata before each use while
// other files declare out of order and must be at the beginning. There are also a number of
// macros which differ from one file to the next due to the method of obtaining the result, such
// as these below. Because of this, there is a theory (Two Team Theory) that states that these
// programming projects had more than 1 "programming team" which utilized different macros for
// each of the files that were worked on.
#define T1_READ_8(ptr)  ((ptr)[0])
#define T1_READ_16(ptr) ((ptr)[0] | ((ptr)[1] << 8))
#define T1_READ_32(ptr) ((ptr)[0] | ((ptr)[1] << 8) | ((ptr)[2] << 16) | ((ptr)[3] << 24))
#define T1_READ_PTR(ptr) (u8 *) T1_READ_32(ptr)

// T2_READ_8 is a duplicate to remain consistent with each group.
#define T2_READ_8(ptr)  ((ptr)[0])
#define T2_READ_16(ptr) ((ptr)[0] + ((ptr)[1] << 8))
#define T2_READ_32(ptr) ((ptr)[0] + ((ptr)[1] << 8) + ((ptr)[2] << 16) + ((ptr)[3] << 24))
#define T2_READ_PTR(ptr) (void *) T2_READ_32(ptr)

// Macros for checking the joypad
#define TEST_BUTTON(field, button) ((field) & (button))
#define JOY_NEW(button) TEST_BUTTON(gMain.newKeys,  button)
#define JOY_HELD(button)  TEST_BUTTON(gMain.heldKeys, button)
#define JOY_HELD_RAW(button) TEST_BUTTON(gMain.heldKeysRaw, button)
#define JOY_REPEAT(button) TEST_BUTTON(gMain.newAndRepeatedKeys, button)

#define S16TOPOSFLOAT(val)   \
({                           \
    s16 v = (val);           \
    float f = (float)v;      \
    if(v < 0) f += 65536.0f; \
    f;                       \
})

#define DIV_ROUND_UP(val, roundBy)(((val) / (roundBy)) + (((val) % (roundBy)) ? 1 : 0))

#define ROUND_BITS_TO_BYTES(numBits) DIV_ROUND_UP(numBits, 8)

// NUM_DEX_FLAG_BYTES allocates more flags than it needs to, as NUM_SPECIES includes the "old unown"
// values that don't appear in the Pokedex. NATIONAL_DEX_COUNT does not include these values.
#define NUM_DEX_FLAG_BYTES ROUND_BITS_TO_BYTES(NUM_SPECIES)
#define NUM_FLAG_BYTES ROUND_BITS_TO_BYTES(FLAGS_COUNT)
#define NUM_TRENDY_SAYING_BYTES ROUND_BITS_TO_BYTES(NUM_TRENDY_SAYINGS)

// This returns the number of arguments passed to it (up to 8).
#define NARG_8(...) NARG_8_(_, ##__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define NARG_8_(_, a, b, c, d, e, f, g, h, N, ...) N

#define CAT(a, b) CAT_(a, b)
#define CAT_(a, b) a ## b

// This produces an error at compile-time if expr is zero.
// It looks like file.c:line: size of array `id' is negative
#define STATIC_ASSERT(expr, id) typedef char id[(expr) ? 1 : -1];

struct Coords8
{
    s8 x;
    s8 y;
};

struct UCoords8
{
    u8 x;
    u8 y;
};

struct Coords16
{
    s16 x;
    s16 y;
};

struct UCoords16
{
    u16 x;
    u16 y;
};

struct Coords32
{
    s32 x;
    s32 y;
};

struct UCoords32
{
    u32 x;
    u32 y;
};

struct Time
{
#include "lu/generated/struct-members/Time.members.h"
};

struct Pokedex
{
#include "lu/generated/struct-members/Pokedex.members.h"
};

struct PokemonJumpRecords
{
#include "lu/generated/struct-members/PokemonJumpRecords.members.h"
};

struct BerryPickingResults
{
#include "lu/generated/struct-members/BerryPickingResults.members.h"
};

struct PyramidBag
{
#include "lu/generated/struct-members/PyramidBag.members.h"
};

struct BerryCrush
{
#include "lu/generated/struct-members/BerryCrush.members.h"
};

struct ApprenticeMon
{
#include "lu/generated/struct-members/ApprenticeMon.members.h"
};

// This is for past players Apprentices or Apprentices received via Record Mix.
// For the current Apprentice, see struct PlayersApprentice
struct Apprentice
{
#include "lu/generated/struct-members/Apprentice.members.h"
};

struct BattleTowerPokemon
{
#include "lu/generated/struct-members/BattleTowerPokemon.members.h"
};

struct EmeraldBattleTowerRecord
{
#include "lu/generated/struct-members/EmeraldBattleTowerRecord.members.h"
};

struct BattleTowerInterview
{
#include "lu/generated/struct-members/BattleTowerInterview.members.h"
};

struct BattleTowerEReaderTrainer
{
#include "lu/generated/struct-members/BattleTowerEReaderTrainer.members.h"
};

// For displaying party information on the player's Battle Dome tourney page
struct DomeMonData
{
#include "lu/generated/struct-members/DomeMonData.members.h"
};

struct RentalMon
{
#include "lu/generated/struct-members/RentalMon.members.h"
};

struct BattleDomeTrainer
{
#include "lu/generated/struct-members/BattleDomeTrainer.members.h"
};

#define DOME_TOURNAMENT_TRAINERS_COUNT 16
#define BATTLE_TOWER_RECORD_COUNT 5

struct BattleFrontier
{
#include "lu/generated/struct-members/BattleFrontier.members.h"
};

struct ApprenticeQuestion
{
#include "lu/generated/struct-members/ApprenticeQuestion.members.h"
};

struct PlayersApprentice
{
#include "lu/generated/struct-members/PlayersApprentice.members.h"
};

struct RankingHall1P
{
#include "lu/generated/struct-members/RankingHall1P.members.h"
};

struct RankingHall2P
{
#include "lu/generated/struct-members/RankingHall2P.members.h"
};

struct SaveBlock2
{
#include "lu/generated/struct-members/SaveBlock2.members.h"
}; // sizeof=0xF2C

extern struct SaveBlock2 *gSaveBlock2Ptr;

struct SecretBaseParty
{
#include "lu/generated/struct-members/SecretBaseParty.members.h"
};

struct SecretBase
{
#include "lu/generated/struct-members/SecretBase.members.h"
};

#include "constants/game_stat.h"
#include "global.fieldmap.h"
#include "global.berry.h"
#include "global.tv.h"
#include "pokemon.h"

struct WarpData
{
#include "lu/generated/struct-members/WarpData.members.h"
};

struct ItemSlot
{
#include "lu/generated/struct-members/ItemSlot.members.h"
};

struct Pokeblock
{
#include "lu/generated/struct-members/Pokeblock.members.h"
};

struct Roamer
{
#include "lu/generated/struct-members/Roamer.members.h"
};

struct RamScriptData
{
#include "lu/generated/struct-members/RamScriptData.members.h"
};

struct RamScript
{
#include "lu/generated/struct-members/RamScript.members.h"
};

// See dewford_trend.c
struct DewfordTrend
{
#include "lu/generated/struct-members/DewfordTrend.members.h"
}; /*size = 0x8*/

struct MauvilleManCommon
{
    u8 id;
};

struct MauvilleManBard
{
    /*0x00*/ u8 id;
    /*0x01*/ //u8 padding1;
    /*0x02*/ u16 songLyrics[BARD_SONG_LENGTH];
    /*0x0E*/ u16 temporaryLyrics[BARD_SONG_LENGTH];
    /*0x1A*/ u8 playerName[PLAYER_NAME_LENGTH + 1];
    /*0x22*/ u8 filler_2DB6[0x3];
    /*0x25*/ u8 playerTrainerId[TRAINER_ID_LENGTH];
    /*0x29*/ bool8 hasChangedSong;
    /*0x2A*/ u8 language;
    /*0x2B*/ //u8 padding2;
}; /*size = 0x2C*/

struct MauvilleManStoryteller
{
    u8 id;
    bool8 alreadyRecorded;
    u8 filler2[2];
    u8 gameStatIDs[NUM_STORYTELLER_TALES];
    u8 trainerNames[NUM_STORYTELLER_TALES][PLAYER_NAME_LENGTH];
    u8 statValues[NUM_STORYTELLER_TALES][4];
    u8 language[NUM_STORYTELLER_TALES];
};

struct MauvilleManGiddy
{
    /*0x00*/ u8 id;
    /*0x01*/ u8 taleCounter;
    /*0x02*/ u8 questionNum;
    /*0x03*/ //u8 padding1;
    /*0x04*/ u16 randomWords[GIDDY_MAX_TALES];
    /*0x18*/ u8 questionList[GIDDY_MAX_QUESTIONS];
    /*0x20*/ u8 language;
    /*0x21*/ //u8 padding2;
}; /*size = 0x2C*/

struct MauvilleManHipster
{
    u8 id;
    bool8 taughtWord;
    u8 language;
};

struct MauvilleOldManTrader
{
    u8 id;
    u8 decorations[NUM_TRADER_ITEMS];
    u8 playerNames[NUM_TRADER_ITEMS][11];
    u8 alreadyTraded;
    u8 language[NUM_TRADER_ITEMS];
};

typedef union OldMan
{
    struct MauvilleManCommon common;
    struct MauvilleManBard bard;
    struct MauvilleManGiddy giddy;
    struct MauvilleManHipster hipster;
    struct MauvilleOldManTrader trader;
    struct MauvilleManStoryteller storyteller;
    u8 filler[0x40];
} OldMan;

#define LINK_B_RECORDS_COUNT 5

struct LinkBattleRecord
{
#include "lu/generated/struct-members/LinkBattleRecord.members.h"
};

struct LinkBattleRecords
{
#include "lu/generated/struct-members/LinkBattleRecords.members.h"
};

struct RecordMixingGiftData
{
#include "lu/generated/struct-members/RecordMixingGiftData.members.h"
};

struct RecordMixingGift
{
#include "lu/generated/struct-members/RecordMixingGift.members.h"
};

struct ContestWinner
{
#include "lu/generated/struct-members/ContestWinner.members.h"
};

struct Mail
{
#include "lu/generated/struct-members/Mail.members.h"
};

struct DaycareMail
{
#include "lu/generated/struct-members/DaycareMail.members.h"
};

struct DaycareMon
{
#include "lu/generated/struct-members/DaycareMon.members.h"
};

struct DayCare
{
#include "lu/generated/struct-members/DayCare.members.h"
};

struct LilycoveLadyQuiz
{
    /*0x000*/ u8 id;
    /*0x001*/ u8 state;
    /*0x002*/ u16 question[QUIZ_QUESTION_LEN];
    /*0x014*/ u16 correctAnswer;
    /*0x016*/ u16 playerAnswer;
    /*0x018*/ u8 playerName[PLAYER_NAME_LENGTH + 1];
    /*0x020*/ u16 playerTrainerId[TRAINER_ID_LENGTH];
    /*0x028*/ u16 prize;
    /*0x02A*/ bool8 waitingForChallenger;
    /*0x02B*/ u8 questionId;
    /*0x02C*/ u8 prevQuestionId;
    /*0x02D*/ u8 language;
};

struct LilycoveLadyFavor
{
    /*0x000*/ u8 id;
    /*0x001*/ u8 state;
    /*0x002*/ bool8 likedItem;
    /*0x003*/ u8 numItemsGiven;
    /*0x004*/ u8 playerName[PLAYER_NAME_LENGTH + 1];
    /*0x00C*/ u8 favorId;
    /*0x00D*/ //u8 padding1;
    /*0x00E*/ u16 itemId;
    /*0x010*/ u16 bestItem;
    /*0x012*/ u8 language;
    /*0x013*/ //u8 padding2;
};

struct LilycoveLadyContest
{
    /*0x000*/ u8 id;
    /*0x001*/ bool8 givenPokeblock;
    /*0x002*/ u8 numGoodPokeblocksGiven;
    /*0x003*/ u8 numOtherPokeblocksGiven;
    /*0x004*/ u8 playerName[PLAYER_NAME_LENGTH + 1];
    /*0x00C*/ u8 maxSheen;
    /*0x00D*/ u8 category;
    /*0x00E*/ u8 language;
};

typedef union // 3b58
{
    struct LilycoveLadyQuiz quiz;
    struct LilycoveLadyFavor favor;
    struct LilycoveLadyContest contest;
    u8 id;
    u8 filler[0x40];
} LilycoveLady;

struct WaldaPhrase
{
#include "lu/generated/struct-members/WaldaPhrase.members.h"
};

struct TrainerNameRecord
{
#include "lu/generated/struct-members/TrainerNameRecord.members.h"
};

struct TrainerHillSave
{
#include "lu/generated/struct-members/TrainerHillSave.members.h"
};

struct WonderNewsMetadata
{
#include "lu/generated/struct-members/WonderNewsMetadata.members.h"
};

struct WonderNews
{
#include "lu/generated/struct-members/WonderNews.members.h"
};

struct WonderCard
{
#include "lu/generated/struct-members/WonderCard.members.h"
};

struct WonderCardMetadata
{
#include "lu/generated/struct-members/WonderCardMetadata.members.h"
};

struct MysteryGiftSave
{
#include "lu/generated/struct-members/MysteryGiftSave.members.h"
}; // 0x36C 0x3598

// For external event data storage. The majority of these may have never been used.
// In Emerald, the only known used fields are the PokeCoupon and BoxRS ones, but hacking the distribution discs allows Emerald to receive events and set the others
struct ExternalEventData
{
#include "lu/generated/struct-members/ExternalEventData.members.h"
} __attribute__((packed)); /*size = 0x14*/

// For external event flags. The majority of these may have never been used.
// In Emerald, Jirachi cannot normally be received, but hacking the distribution discs allows Emerald to receive Jirachi and set the flag
struct ExternalEventFlags
{
#include "lu/generated/struct-members/ExternalEventFlags.members.h"
} __attribute__((packed));/*size = 0x15*/

struct SaveBlock1
{
#include "lu/generated/struct-members/SaveBlock1.members.h"
};

extern struct SaveBlock1* gSaveBlock1Ptr;

struct MapPosition
{
    s16 x;
    s16 y;
    s8 elevation;
};

#endif // GUARD_GLOBAL_H
