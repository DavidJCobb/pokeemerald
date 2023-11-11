#include "lu/custom_game_option_script_helpers.h"
#include "lu/custom_game_options.h"

bool8 GetCustomGameOptionBool(u8 id) {
   switch (id) {
      case CGOPTION_BOOL_GRANT_CATCH_XP:
         return gCustomGameOptions.enable_catch_exp;
   }
   return FALSE;
}

bool8 GetCustomGameStateBool(u8 id) {
   switch (id) {
      case CGSTATE_BOOL_HAS_EVER_HAD_POKE_BALLS:
         return gCustomGameSavestate.has_ever_had_poke_balls;
   }
   return FALSE;
}