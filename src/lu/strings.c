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
// NEW GAME SUB-OPTIONS
//

const u8 gText_lu_NewGame_Vanilla[] = _("Vanilla Options");
const u8 gText_lu_NewGame_Enhanced[] = _("Enhanced Options");
const u8 gText_lu_NewGame_CustomGame[] = _("Custom Game");

//
// CUSTOM GAME OPTIONS MENU
//

const u8 gText_lu_CGO_menuTitle[] = _("CUSTOM GAME OPTIONS");

const u8 gText_lu_CGOption_startWithRunningShoes[] = _("Start w/ Running");

const u8 gText_lu_CGOptionValues_common_Disabled[] = _("Enabled");
const u8 gText_lu_CGOptionValues_common_Enabled[]  = _("Disabled");