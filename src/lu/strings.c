#include "global.h"
#include "lu/strings.h"

// Options Menu: Option names
const u8 gText_lu_option_running[] = _("RUNNING");

// Options Menu: option value labels
// Code for this menu assumes that the strings begin with two format codes. It first 
// copies the strings to a buffer; then overwrites the format codes to distinguish 
// the selected value from the others. Option values have a maximum length of 16, 
// including the format codes (which are each multiple bytes) and terminator -- so, 
// a maximum text length of 11 or so.
const u8 gText_lu_option_running_vanilla[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}HOLD");
const u8 gText_lu_option_running_toggle[]  = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}TOGGLE");

//

const u8 gText_lu_MainMenuCustomGameOptions[] = _("CUSTOM GAME OPTIONS");

//
// NEW GAME SUB-OPTIONS
//

const u8 gText_lu_NewGame_Vanilla[] = _("Vanilla Options");
const u8 gText_lu_NewGame_Enhanced[] = _("Enhanced Options");
const u8 gText_lu_NewGame_CustomGame[] = _("Custom Game");

//
// CUSTOM GAME OPTIONS MENU
//

const u8 gText_lu_CGO_keybinds[] = _("{DPAD_UPDOWN}PICK  {DPAD_LEFTRIGHT}CHANGE  {A_BUTTON}OK  {B_BUTTON}BACK  {L_BUTTON}{R_BUTTON}HELP");

const u8 gText_lu_CGO_menuTitle[] = _("CUSTOM GAME OPTIONS");

const u8 gText_lu_CGOptionName_StartWithRunningShoes[] = _("Start w/ running");
const u8 gText_lu_CGOptionHelp_StartWithRunningShoes[] = _("In the original Pokémon Emerald, you could only run on the overworld after receiving your first Pokémon, your Pokédex, and the Running Shoes. This option can be used to enable running from the moment you start the game.");

const u8 gText_lu_CGOptionCategory_OverworldPoison[] = _("Overworld poison damage");
//
const u8 gText_lu_CGOptionName_OverworldPoison_Interval[] = _("Damage interval (steps)");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Interval[] = _("When moving on the overworld, poisoned Pokémon will take damage with every few steps you take. This option can be used to increase or decrease that number of steps, or to disable poison damage on the overworld entirely.\n\nBy default, Pokémon take damage after every 4 steps.");
//
const u8 gText_lu_CGOptionName_OverworldPoison_Damage[] = _("Damage dealt");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Damage[] = _("When moving on the overworld, poisoned Pokémon will take damage with every few steps you take. This option can be used to change the amount of damage dealt at one time.\n\nBy default, Pokémon take 1 HP of damage every time they are hurt by poison on the overworld.");

const u8 gText_lu_CGOptionValues_common_Default[] = _("Default");
const u8 gText_lu_CGOptionValues_common_Disabled[] = _("Disabled");
const u8 gText_lu_CGOptionValues_common_Enabled[]  = _("Enabled");
const u8 gText_lu_CGOptionValues_common_None[]  = _("None");