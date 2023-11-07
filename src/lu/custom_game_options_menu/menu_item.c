#include "lu/custom_game_options_menu/menu_item.h"
#include "lu/strings.h"
#include "data.h" // gSpeciesNames
#include "constants/species.h" // NUM_SPECIES and similar

bool8 MenuItemIsListTerminator(const struct CGOptionMenuItem* item) {
   if (item->name == NULL)
      return TRUE;
   if (item->flags & (1 << MENUITEM_FLAG_END_OF_LIST_SENTINEL))
      return TRUE;
   return FALSE;
}
u8 GetMenuItemListCount(const struct CGOptionMenuItem* items) {
   u8 i;
   for(i = 0; !MenuItemIsListTerminator(&items[i]); ++i) {
      ;
   }
   return i;
}

u16 GetOptionValue(const struct CGOptionMenuItem* item) {
   switch (item->value_type) {
      case VALUE_TYPE_BOOL8:
         return *item->target.as_bool8 ? 1 : 0;
      case VALUE_TYPE_U8:
         return *item->target.as_u8;
      case VALUE_TYPE_U16:
      case VALUE_TYPE_POKEMON_SPECIES:
         return *item->target.as_u16;
   }
   return 0;
}
void SetOptionValue(const struct CGOptionMenuItem* item, u16 value) {
   switch (item->value_type) {
      case VALUE_TYPE_BOOL8:
         *item->target.as_bool8 = value ? 1 : 0;
         break;
      case VALUE_TYPE_U8:
         *item->target.as_u8 = value;
         break;
      case VALUE_TYPE_U16:
      case VALUE_TYPE_POKEMON_SPECIES:
         *item->target.as_u16 = value;
         break;
   }
}

const u8* GetOptionValueName(const struct CGOptionMenuItem* item, u16 value) {
   if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
      return NULL;
   }
   if (item->flags & (1 << MENUITEM_FLAG_IS_ENUM)) {
      return item->values.named.name_strings[value];
   }
   
   if (item->value_type == VALUE_TYPE_BOOL8) {
      if (GetOptionValue(item) == 0) {
         return gText_lu_CGOptionValues_common_Disabled;
      }
      return gText_lu_CGOptionValues_common_Enabled;
   }
   
   if (item->flags & (1 << MENUITEM_FLAG_0_MEANS_DISABLED)) {
      return gText_lu_CGOptionValues_common_Disabled;
   }
   if (item->flags & (1 << MENUITEM_FLAG_0_MEANS_DEFAULT)) {
      return gText_lu_CGOptionValues_common_Default;
   }
   if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      u16 species = GetOptionValue(item);
      if (species == 0) {
         return gText_lu_CGOptionValues_common_None;
      }
      if (species >= NUM_SPECIES) {
         return gSpeciesNames[0];
      }
      return gSpeciesNames[species];
   }
   
   return NULL;
}
u8 GetOptionValueCount(const struct CGOptionMenuItem* item) {
   if (item->flags & (1 << MENUITEM_FLAG_IS_SUBMENU)) {
      return 0;
   }
   if (item->flags & (1 << MENUITEM_FLAG_IS_ENUM)) {
      return item->values.named.count;
   }
   if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      return 0; // TODO
   }
   switch (item->value_type) {
      case VALUE_TYPE_BOOL8:
         return 2; // Disabled, Enabled
      case VALUE_TYPE_U8:
      case VALUE_TYPE_U16:
         return item->values.integral.max - item->values.integral.min;
   }
   return 0;
}
u16 GetOptionMinValue(const struct CGOptionMenuItem* item) {
   if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      if (item->flags & (1 << MENUITEM_FLAG_POKEMON_SPECIES_ALLOW_0)) {
         return 0;
      }
      return 1;
   }
   switch (item->value_type) {
      case VALUE_TYPE_U8:
      case VALUE_TYPE_U16:
         return item->values.integral.min;
   }
   return 0;
}
void CycleOptionSelectedValue(const struct CGOptionMenuItem* item, s8 by) {
   u16 selection;
   
   selection = GetOptionValue(item);
   if (item->value_type == VALUE_TYPE_BOOL8) {
      selection = !(selection & 1);
   } else if (item->value_type == VALUE_TYPE_POKEMON_SPECIES) {
      return;
   } else {
      u8  value_count;
      u16 minimum;
      
      value_count = GetOptionValueCount(item);
      minimum     = GetOptionMinValue(item);
      
      if (by < 0) {
         if (selection - minimum < -by) {
            selection = value_count - (-by - selection);
         } else {
            selection += by;
         }
      } else if (by > 0) {
         selection += by;
         selection %= value_count;
      }
      if (selection < minimum) {
         selection = minimum;
      }
   }
   
   SetOptionValue(item, selection);
}
