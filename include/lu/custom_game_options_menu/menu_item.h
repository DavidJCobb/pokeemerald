#ifndef GUARD_LU_CUSTOM_GAME_OPTIONS_MENU_ITEM
#define GUARD_LU_CUSTOM_GAME_OPTIONS_MENU_ITEM

#include "global.h"

enum { // CGOptionMenuItem::value_type
   VALUE_TYPE_NONE, // e.g. "Cancel"
   VALUE_TYPE_U8,
   VALUE_TYPE_U16,
   VALUE_TYPE_BOOL8,
   
   VALUE_TYPE_POKEMON_SPECIES, // 0 = None; defined species start at 1. Do not use the "is enum" flag with this type.
};

enum { // CGOptionMenuItem::flags
   MENUITEM_FLAG_END_OF_LIST_SENTINEL,
   MENUITEM_FLAG_IS_ENUM,
   MENUITEM_FLAG_IS_SUBMENU,
   MENUITEM_FLAG_0_MEANS_DISABLED, // Display zero as "Disabled" instead of "0"
   MENUITEM_FLAG_0_MEANS_DEFAULT,  // Display zero as "Default" instead of "0"
   MENUITEM_FLAG_PERCENTAGE,
   MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0, // Allow zero as a Pokemon species number (displays as "None" by default)
};

struct CGOptionMenuItem {
   const u8* name; // == NULL for (sub)menu end sentinel
   const u8* help_string;
   
   u8 flags;
   u8 value_type;
   union {
      struct {
         const u8* const * name_strings;
         const u8* const * help_strings;
         u8 count;
      } named; // used if MENUITEM_FLAG_IS_ENUM is set
      struct {
         s16 min;
         s16 max;
      } integral; // used if MENUITEM_FLAG_IS_ENUM is not set and the `value_type` is marked as u8 or u16
   } values;
   union {
      bool8* as_bool8;
      u8*    as_u8;
      u16*   as_u16;
      const struct CGOptionMenuItem* submenu;
   } target;
};

bool8 MenuItemIsListTerminator(const struct CGOptionMenuItem* item);
u8    GetMenuItemListCount(const struct CGOptionMenuItem* items);

u16  GetOptionValue(const struct CGOptionMenuItem* item);
void SetOptionValue(const struct CGOptionMenuItem* item, u16 value);

const u8* GetOptionValueName(const struct CGOptionMenuItem* item, u16 value);
u16       GetOptionValueCount(const struct CGOptionMenuItem* item);
u16       GetOptionMinValue(const struct CGOptionMenuItem* item);

void CycleOptionSelectedValue(const struct CGOptionMenuItem* item, s8 by);

const u8* GetRelevantHelpText(const struct CGOptionMenuItem* item);

#endif