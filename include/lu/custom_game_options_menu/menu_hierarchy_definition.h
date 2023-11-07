#include "lu/custom_game_options_menu/menu_item.h"
#include "lu/strings.h"

#define END_OF_LIST_SENTINEL { \
   .name        = NULL, \
   .help_string = NULL, \
   .flags       = (1 << MENUITEM_FLAG_END_OF_LIST_SENTINEL), \
   .value_type  = VALUE_TYPE_NONE, \
   .target      = NULL, \
}

//

static const struct CGOptionMenuItem sBattleOptions[] = {
   {  // Scale accuracy by player
      .name        = gText_lu_CGOptionName_BattlesScaleAccuracyPlayer,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleAccuracyPlayer,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.scale_battle_accuracy_player
      }
   },
   {  // Scale accuracy by enemy
      .name        = gText_lu_CGOptionName_BattlesScaleAccuracyEnemy,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleAccuracyEnemy,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.scale_battle_accuracy_enemy
      }
   },
   {  // Scale accuracy by ally
      .name        = gText_lu_CGOptionName_BattlesScaleAccuracyAlly,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleAccuracyAlly,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.scale_battle_accuracy_ally
      }
   },
   {  // Scale damage by player
      .name        = gText_lu_CGOptionName_BattlesScaleDamagePlayer,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleDamagePlayer,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.scale_battle_damage_dealt_by_player
      }
   },
   {  // Scale damage by enemy
      .name        = gText_lu_CGOptionName_BattlesScaleDamageEnemy,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleDamageEnemy,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.scale_battle_damage_dealt_by_enemy
      }
   },
   {  // Scale damage by ally
      .name        = gText_lu_CGOptionName_BattlesScaleDamageAlly,
      .help_string = gText_lu_CGOptionHelp_BattlesScaleDamageAlly,
      .flags       = (1 << MENUITEM_FLAG_PERCENTAGE),
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 0,
            .max = 5000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.scale_battle_damage_dealt_by_ally
      }
   },
   END_OF_LIST_SENTINEL,
};

static const struct CGOptionMenuItem sOverworldPoisonOptions[] = {
   {  // Interval
      .name        = gText_lu_CGOptionName_OverworldPoison_Interval,
      .help_string = gText_lu_CGOptionHelp_OverworldPoison_Interval,
      .flags       = (1 << MENUITEM_FLAG_0_MEANS_DISABLED),
      .value_type = VALUE_TYPE_U8,
      .values = {
         .integral = {
            .min = 0,
            .max = 60,
         }
      },
      .target = {
         .as_u8 = &sTempOptions.overworld_poison_interval
      }
   },
   {  // Damage
      .name        = gText_lu_CGOptionName_OverworldPoison_Damage,
      .help_string = gText_lu_CGOptionHelp_OverworldPoison_Damage,
      .flags       = 0,
      .value_type = VALUE_TYPE_U16,
      .values = {
         .integral = {
            .min = 1,
            .max = 2000,
         }
      },
      .target = {
         .as_u16 = &sTempOptions.overworld_poison_damage
      }
   },
   END_OF_LIST_SENTINEL,
};

static const struct CGOptionMenuItem sTopLevelMenu[] = {
   {  // Start with running shoes
      .name        = gText_lu_CGOptionName_StartWithRunningShoes,
      .help_string = gText_lu_CGOptionHelp_StartWithRunningShoes,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.start_with_running_shoes
      }
   },
   {  // Allow running indoors
      .name        = gText_lu_CGOptionName_AllowRunningIndoors,
      .help_string = NULL,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.can_run_indoors
      }
   },
   {  // Allow biking indoors
      .name        = gText_lu_CGOptionName_AllowBikingIndoors,
      .help_string = NULL,
      .flags       = 0,
      .value_type = VALUE_TYPE_BOOL8,
      .target = {
         .as_bool8 = &sTempOptions.can_bike_indoors
      }
   },
   {  // SUBMENU: Battle options
      .name        = gText_lu_CGOptionCategoryName_Battles,
      .help_string = NULL,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sBattleOptions
      },
   },
   {  // SUBMENU: Overworld poison
      .name        = gText_lu_CGOptionCategoryName_OverworldPoison,
      .help_string = gText_lu_CGOptionCategoryHelp_OverworldPoison,
      .flags       = (1 << MENUITEM_FLAG_IS_SUBMENU),
      .value_type = VALUE_TYPE_NONE,
      .target = {
         .submenu = sOverworldPoisonOptions
      }
   },
   END_OF_LIST_SENTINEL,
};
