#ifndef GUARD_LU_CUSTOM_GAME_OPTIONS
#define GUARD_LU_CUSTOM_GAME_OPTIONS
#include "lu/bitstreams.h"

enum {
   CustomGame_SingleSpeciesRun_CatchMode_OnlyCatchTargetSpecies,
   CustomGame_SingleSpeciesRun_CatchMode_PolymorphAllCaught,
};

enum {
   // Double Battles function as normal.
   CustomGame_DoubleBattles_Normal,
   
   // Double Battles function as normal, except that you can willingly enter a 1v2.
   CustomGame_DoubleBattles_Permissive,
   
   // All battles are Double Battles.
   CustomGame_DoubleBattles_Forced,
   
   // No battles are Double Battles.
   CustomGame_DoubleBattles_Disabled,
};

struct CustomGameScaleAndClamp {
   u16 scale; // percentage, i.e. 100 = 100%
   u16 min;
   u16 max;   // 0xFFFF = no max
};

#define UNIMPLEMENTED_CUSTOM_GAME_OPTION

// Track current values of Custom Game options. Intended to be serialized after SaveBlock2.
extern struct CustomGameOptions {
   bool8 start_with_running_shoes;
   
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_wild_encounter_rate;
   
   // Scale damage dealt by different participants in a battle.
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_battle_damage_dealt_by_player;
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_battle_damage_dealt_by_enemy;
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_battle_damage_dealt_by_ally;
   
   // Scale the move accuracy of different participants in a battle.
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_battle_accuracy_player;
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_battle_accuracy_enemy;
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_battle_accuracy_ally;
   
   UNIMPLEMENTED_CUSTOM_GAME_OPTION struct CustomGameScaleAndClamp player_money_loss_on_defeat;
   UNIMPLEMENTED_CUSTOM_GAME_OPTION struct CustomGameScaleAndClamp player_money_gain_on_victory;
   
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_shop_prices_sell;
   UNIMPLEMENTED_CUSTOM_GAME_OPTION u16 scale_shop_prices_buy;
   
   u8  overworld_poison_interval; // step count; 0 to disable; range [0, 60]
   u16 overworld_poison_damage;   // damage taken per poison field effect; range [1, 2000]
   
   // generational battle rules:
   UNIMPLEMENTED_CUSTOM_GAME_OPTION bool8 no_physical_special_split; // TODO: implement the physical/special split first lol
   
   // Gen V battle improvements:
   UNIMPLEMENTED_CUSTOM_GAME_OPTION bool8 no_inherently_typeless_moves; // Beat Up, Future Sight, and Doom Desire are typed
   UNIMPLEMENTED_CUSTOM_GAME_OPTION bool8 wonder_guard_blocks_typeless; // Wonder Guard blocks typeless moves except Struggle
} gCustomGameOptions;

// Track in-game progress related to custom game options. Intended to be serialized after SaveBlock2.
extern struct CustomGameSavestate {
   
   // Needed for Nuzlocke encounter options: we should not enforce any encounter/catch 
   // limits until the player has obtained at least one of any Poke Ball type.
   bool8 has_ever_had_poke_balls;
   
} gCustomGameSavestate;

#undef UNIMPLEMENTED_CUSTOM_GAME_OPTION

extern void ResetCustomGameOptions(void);
extern void ResetCustomGameSavestate(void);

extern void CustomGames_HandleNewPlaythroughStarted(void);

#endif