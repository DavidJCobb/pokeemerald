#include "global.h"
#include "lu/custom_game_options.h"
#include "event_data.h" // FlagSet

#include "lu/bitstreams.h"

#define CUSTOM_GAME_OPTION_DATA_VERSION 0

EWRAM_DATA struct CustomGameOptions   gCustomGameOptions   = {};
EWRAM_DATA struct CustomGameSavestate gCustomGameSavestate = {};

static void InitializeScaleAndClamp(struct CustomGameScaleAndClamp* v) {
   v->scale = 100;
   v->min   = 0;
   v->max   = 0xFFFF;
}

u16 ApplyCustomGameScale_u16(u16 v, u16 scale) {
   if (scale != 100) {
      v = (u32)v * scale / 100;
   }
   return v;
}
s32 ApplyCustomGameScale_s32(s32 v, u16 scale) {
   if (scale != 100) {
      v = v * scale / 100;
   }
   return v;
}

u16 ApplyCustomGameScaleAndClamp_u16(u16 v, const struct CustomGameScaleAndClamp* params) {
   if (params->scale != 100) {
      u32 scaled = v;
      scaled *= params->scale;
      scaled /= 100;
   }
   if (v < params->min)
      v = params->min;
   else if (v > params->max)
      v = params->max;
   return v;
}

void ResetCustomGameOptions(void) {
   gCustomGameOptions.start_with_running_shoes = FALSE;
   gCustomGameOptions.can_run_indoors          = FALSE;
   gCustomGameOptions.can_bike_indoors         = FALSE;
   
   gCustomGameOptions.scale_wild_encounter_rate = 100;
   
   gCustomGameOptions.scale_battle_damage_dealt_by_player = 100;
   gCustomGameOptions.scale_battle_damage_dealt_by_enemy  = 100;
   gCustomGameOptions.scale_battle_damage_dealt_by_ally   = 100;
   
   gCustomGameOptions.scale_battle_accuracy_player = 100;
   gCustomGameOptions.scale_battle_accuracy_enemy  = 100;
   gCustomGameOptions.scale_battle_accuracy_ally   = 100;
   
   InitializeScaleAndClamp(&gCustomGameOptions.player_money_loss_on_defeat);
   InitializeScaleAndClamp(&gCustomGameOptions.player_money_gain_on_victory);
   
   gCustomGameOptions.scale_shop_prices_sell = 100;
   gCustomGameOptions.scale_shop_prices_buy  = 100;
   
   gCustomGameOptions.overworld_poison_interval = 4; // vanilla value; formerly hardcoded
   gCustomGameOptions.overworld_poison_damage   = 1; // vanilla value; formerly hardcoded
}

void ResetCustomGameSavestate(void) {
   gCustomGameSavestate.has_ever_had_poke_balls = FALSE;
}

void CustomGames_HandleNewPlaythroughStarted(void) {
   ResetCustomGameSavestate();
   
   if (gCustomGameOptions.start_with_running_shoes) {
      FlagSet(FLAG_SYS_B_DASH);
   }
}