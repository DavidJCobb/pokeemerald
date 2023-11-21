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
// MESSAGES
//

const u8 gText_lu_CGRevivesNotAllowedInBattle[] = ("Can't use revives in battle.");

//
// CUSTOM GAME OPTIONS MENU
//

const u8 gText_lu_CGO_KeybindsForItem[]    = _("{DPAD_UPDOWN}PICK  {DPAD_LEFTRIGHT}CHANGE  {B_BUTTON}BACK");
const u8 gText_lu_CGO_KeybindsForHelp[]    = _("{B_BUTTON}RETURN TO MENU");
const u8 gText_lu_CGO_keybindsForSubmenu[] = _("{DPAD_UPDOWN}PICK  {A_BUTTON}ENTER SUBMENU  {B_BUTTON}BACK");
const u8 gText_lu_CGO_keybindFragment_ItemHelp[] = _("  {L_BUTTON}{R_BUTTON}HELP");

const u8 gText_lu_CGO_menuTitle[] = _("CUSTOM GAME OPTIONS");

const u8 gText_lu_CGOptionName_StartWithRunningShoes[] = _("Start w/ running");
const u8 gText_lu_CGOptionHelp_StartWithRunningShoes[] = _("In the original Pokémon Emerald, you could only run on the overworld after receiving your first Pokémon, your Pokédex, and the Running Shoes. This option can be used to enable running from the moment you start the game.");

const u8 gText_lu_CGOptionName_AllowRunningIndoors[] = _("Allow running indoors");

const u8 gText_lu_CGOptionName_AllowBikingIndoors[] = _("Allow biking indoors");

const u8 gText_lu_CGOptionCategoryName_Catching[] = _("Pokémon catching options");

const u8 gText_lu_CGOptionName_CatchEXP[] = _("Catch EXP");
const u8 gText_lu_CGOptionHelp_CatchEXP[] = _("Control whether the player's Pokémon earn EXP when the player catches a Wild Pokémon. EXP earnings are the same as if the Wild Pokémon were defeated in battle.\n\nDefault: Disabled.");

const u8 gText_lu_CGOptionName_CatchRateBase[] = _("Catch rate increase");
const u8 gText_lu_CGOptionHelp_CatchRateBase[] = _("Apply a flat increase to all computed catch rates. Setting this value to 100%, for example, would guarantee catch odds of 100%.\n\nDefault: 0%: catch rates do not receive a flat increase.");

const u8 gText_lu_CGOptionName_CatchRateScale[] = _("Catch rate scale");
const u8 gText_lu_CGOptionHelp_CatchRateScale[] = _("Multiply the computed catch chance when throwing any Poké Ball. This multiplier is applied after all catch calculations are complete.\n\nDefault: 100%.");

const u8 gText_lu_CGOptionCategoryName_Battles[] = _("Battle options");

const u8 gText_lu_CGOptionName_ItemUseInBattles[] = _("Item use");
const u8 gText_lu_CGOptionHelp_ItemUseInBattles[] = _("Control whether the player is allowed to use items while in battle.");
//
const u8 gText_lu_CGOptionValueName_ItemUseInBattles_NoBackfield[] = _("No Backfield");
const u8 gText_lu_CGOptionValueHelp_ItemUseInBattles_NoBackfield[] = _("Control whether the player is allowed to use items while in battle. The “No Backfield” option limits players to using items on Pokémon that are currently in battle; this is the same limit that NPCs have. This limit does not apply to items that can revive a fainted Pokémon; to disable those too, choose “NB + NR”.");
const u8 gText_lu_CGOptionValueName_ItemUseInBattles_NoRevives[] = _("No Revives");
const u8 gText_lu_CGOptionValueHelp_ItemUseInBattles_NoRevives[] = _("Control whether the player is allowed to use items while in battle. The “No Revives” option disallows reviving a fainted Pokémon.");
const u8 gText_lu_CGOptionValueName_ItemUseInBattles_NoBackfieldAndNoRevives[] = _("NB + NR");
const u8 gText_lu_CGOptionValueHelp_ItemUseInBattles_NoBackfieldAndNoRevives[] = _("Control whether the player is allowed to use items while in battle. The “NB + NR” option doesn't allow the player to use items on Pokémon that aren't on the field, even revives.");

const u8 gText_lu_CGOptionName_BattlesScaleDamagePlayer[] = _("Scale damage (player)");
const u8 gText_lu_CGOptionHelp_BattlesScaleDamagePlayer[] = _("Applies a percentage multiplier to damage dealt by the player. This multiplier is applied after all other damage calculations, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleDamageEnemy[] = _("Scale damage (enemies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleDamageEnemy[] = _("Applies a percentage multiplier to damage dealt by the player's opponents. This multiplier is applied after all other damage calculation, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleDamageAlly[] = _("Scale damage (allies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleDamageAlly[] = _("Applies a percentage multiplier to damage dealt by the player's NPC allies, like Steven in Mossdeep City. This multiplier is applied after all other damage calculations, and does not affect NPC AI decision-making.");

const u8 gText_lu_CGOptionName_BattlesScaleAccuracyPlayer[] = _("Scale accuracy (player)");
const u8 gText_lu_CGOptionHelp_BattlesScaleAccuracyPlayer[] = _("Applies a percentage multiplier to the player's accuracy. This multiplier is applied after all other accuracy calculations, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleAccuracyEnemy[] = _("Scale accuracy (enemies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleAccuracyEnemy[] = _("Applies a percentage multiplier to opponents' accuracy. This multiplier is applied after all other accuracy calculations, and does not affect NPC AI decision-making.");
const u8 gText_lu_CGOptionName_BattlesScaleAccuracyAlly[] = _("Scale accuracy (allies)");
const u8 gText_lu_CGOptionHelp_BattlesScaleAccuracyAlly[] = _("Applies a percentage multiplier to NPC allies' accuracy. This multiplier is applied after all other accuracy calculations, and does not affect NPC AI decision-making.");

const u8 gText_lu_CGOptionName_BattlesScaleEXPNormal[] = _("Scale EXP gains (normal)");
const u8 gText_lu_CGOptionHelp_BattlesScaleEXPNormal[] = _("Scale the EXP awarded to the player's non-traded Pokémon.\n\nDefault: 100%.");
const u8 gText_lu_CGOptionName_BattlesScaleEXPTraded[] = _("Scale EXP gains (traded)");
const u8 gText_lu_CGOptionHelp_BattlesScaleEXPTraded[] = _("Scale the EXP awarded to the player's traded Pokémon. Text in the UI will display as normal, describing the EXP earned as “boosted” regardless of the value you set here.\n\nDefault: 150%.");

const u8 gText_lu_CGOptionName_BattlesScaleVictoryPayout[] = _("Money gain on victory");
const u8 gText_lu_CGOptionHelp_BattlesScaleVictoryPayout[] = _("Applies a percentage multiplier to the money the player earns after winning a battle. This only affects the payout received from the opposing trainer; it doesn't affect other income, such as the results of using Pay Day.");

const u8 gText_lu_CGOptionName_MoneyLossOnDefeat[] = _("Money loss on defeat");
const u8 gText_lu_CGOptionHelp_MoneyLossOnDefeat[] = _("In the classic Pokémon games, losing a battle would cost the player exactly half of all the money they were carrying. In FireRed, LeafGreen, and from Gen IV onward, the player's money loss is calculated similarly to NPCs.\n\nDefault: Classic.");
//
const u8 gText_lu_CGOptionValueName_MoneyLossOnDefeat_Classic[] = _("Classic");
const u8 gText_lu_CGOptionValueName_MoneyLossOnDefeat_Modern[] = _("Modern");

const u8 gText_lu_CGOptionCategoryName_OverworldPoison[] = _("Overworld poison damage");
const u8 gText_lu_CGOptionCategoryHelp_OverworldPoison[] = _("On the overworld, poisoned Pokémon take damage after every few steps you take. You can change the number of steps and amount of damage dealt, or disable poison damage on the overworld entirely.");
//
const u8 gText_lu_CGOptionName_OverworldPoison_Interval[] = _("Damage interval (steps)");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Interval[] = _("Poisoned Pokémon take damage after every few steps you take. (Steps are counted even when no one is poisoned.) You can change the number of steps, or disable poison damage on the overworld entirely.\n\nDefault: Pokémon take damage every 4 steps.");
//
const u8 gText_lu_CGOptionName_OverworldPoison_Damage[] = _("Damage dealt");
const u8 gText_lu_CGOptionHelp_OverworldPoison_Damage[] = _("Poisoned Pokémon take damage after every few steps you take. You can change the amount of damage dealt at one time.\n\nDefault: Pokémon take 1 HP of damage at a time.");

const u8 gText_lu_CGOptionValues_common_Default[] = _("Default");
const u8 gText_lu_CGOptionValues_common_Disabled[] = _("Disabled");
const u8 gText_lu_CGOptionValues_common_Enabled[]  = _("Enabled");
const u8 gText_lu_CGOptionValues_common_None[]  = _("None");
const u8 gText_lu_CGOptionValues_common_Random[]  = _("Random");