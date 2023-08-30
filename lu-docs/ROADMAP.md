
== Overall feature work ==

* ✅ Automatic word-wrapping
** ✅ Base implementation
** ⬜ Actually using it *//non-essential*
*** ⬜ `main_menu.c` (Birch intro cutscene)
*** ⬜ `apprentice.c` (Battle Frontier apprentices)
*** ⬜ `field_message_box.c` (overworld messages, incl. dialogue, scripting, etc.)
* ⬜ Options Menu redesign
** ✅ Vertical scrolling
** ✅ Display UI indicating that vertical scrolling is possible
** ⬜ Visual redesign
*** ⬜ No window frames; mimic UI of FR/LG help menus, for more space
**** *// the trouble with this is, how do we display a preview for the currently selected frame option?*
*** ⬜ Action bar at the bottom (e.g. "[A] Cycle [D-Pad H] Change [D-Pad V] Scroll [B] Exit")
*** ⬜ Description bar at bottom, showing behavior of currently highlighted option and/or currently selected value
* ⬜ Option to press B to toggle running, instead of having to hold it
** ✅ Options Menu item
** ✅ Savedata field (in save block 2)
** ✅ Overworld behavior
** ⬜ HUD of some kind, shown on the overworld when running is toggled on
*** *// field moves display effects overtop the screen. can we co-opt however that's done?*
* ⬜ Battle flow improvements
** ⬜ Coalesce multiple messages together
*** ✅ Depends on: automatic word-wrapping
** ⬜ Concurrent text and animations for stat changes
** ⬜ Use non-"it" pronouns for Pokemon
* ⬜ Overworld pacing improvements
** ⬜ Speed up fade-in and other transitions during the Professor Birch intro
** ⬜ Coalesce overworld system messages
*** *// e.g. "PLAYER found an ITEM, and put it in the DOODAD POCKET." instead of two textboxes. the automatic word-wrapping code will help with this!*
* ⬜ Savedata format optimizations
** ⬜ Player inventory
** ⬜ Can we write serialization/parse functions to store SaveBlock1 in a bitpacked format?
*** *// i mean, we* can*, but can we do so efficiently and safely?*
** ⬜ Compile-time flags to control presence/absence of certain savedata (and game features generally)
*** ⬜ Enigma berry data
*** ⬜ Mystery Gift data
* ⬜ Unlimited player inventory + no PC item storage
** ⬜ Depends on: savedata format optimizations: player inventory
* ⬜ Increase Pokemon name length to 12 characters
* ⬜ Increase player name length to 12 characters
** ⬜ Depends on: savedata format optimizations (each byte of length increase adds 126 bytes to total savedata size)
* ⬜ Character customization
** ⬜ Separate player pronouns from body, and add a neutral pronoun option
*** *// actually, i can't find any cases in the vanilla game where the player is referred to by pronoun, aside from Japanese honorifics*
** ⬜ Trainer sprite palette changes
** ⬜ Trainer sprite pose changes
* ⬜ Hotkey menu instead of just one single hotkeyed key item
* ⬜ Text patches
** ⬜ Replace all-caps with color changes
*** ⬜ Identifiers
**** *// since we have source-level access, should be trivial to do this by running a script on the source files*
**** ⬜ Ability names, incl. hardcoded mentions in battle strings
**** ⬜ Item names
**** ⬜ Move names
**** ⬜ Pokemon species names
**** ⬜ Trainer names
*** ⬜ Dialogue
**** ⬜ System messages (e.g. "PLAYER picked up an ITEM")
**** ⬜ Pokedex entries
** ⬜ Investigate making multiple-choice menus scriptable
*** *// `Task_ShowScrollableMultichoice` hardcodes several choice menus, e.g. crafting ash flutes*
** ⬜ Add a UI widget to overworld text to show the speaking NPC's name
** ⬜ Investigate adding parameterized placeholder codes for identifiers, to aid content creation and localization
*** ⬜ Feasibility study: how much of a reduction in filesize would we gain? Compare to both "PIKACHU" and "Pikachu"-but-with-color-format-codes.
*** ⬜ Ability name
*** ⬜ Pokemon species name
*** ⬜ Item name
*** ⬜ Move name
** ⬜ Investigate adding support for italicized text
* ⬜ Trainer sprites for NPCs expanded on in the remakes
** ⬜ Courtney (Magma Admin)
*** *// iirc, Courtney is not fought in vanilla Emerald. having a sprite on hand could still be useful.*
** ⬜ Matt (Aqua Admin)
** ⬜ Shelley (Aqua Admin)
** ⬜ Tabitha (Magma Admin)
*** *// although rare, Tabitha can be used as a guy's name. this is one of those cases*
* ⬜ The same large-scale major changes tons of people have already made
** *// don't know if or when I want to incorporate other code changes or feature branches, or collab with other authors. worth saving these kinds of modernization updates 'til later -- 'til i have any progress of my own worth caring about.*
** ⬜ Fairy type
** ⬜ Physical/special split
** ⬜ Pokedex expansion
*** *// requires tons of sprites...*
** ⬜ Overworld follower Pokemon
*** *// requires tons of sprites...*
* ⬜ Custom Game Options
** ⬜ Depends on: savedata format optimizations
*** *// CG custom data will be bitpacked and versioned*
** ⬜ Savedata
*** *// Prior to savedata optimization, we can just... write this wherever, pretty much. We can jam it at the end of either save block, though we lose forward-compatibility (which is fine for internal testing/development). We already have a singleton struct in memory to hold these prefs, though it's not ASLR'd, so we should modify the save routines to write it after the saveblocks rather than appending it to the saveblock structs themselves.*
** ⬜ Apply ASLR to the in-memory CustomGameOptions struct, just like the game does for the two saveblocks.
** ⬜ Custom Game Options menu
*** ✅ Basic implementation (NOTE: untested)
*** ⬜ Show it after selecting "New Game"
*** ⬜ Functionality for nested menus
** ⬜ Toggle editing these options after starting the playthrough
** ⬜ Pokemon availability options
*** ⬜ Override starters
*** ⬜ Single-species run
*** ⬜ Full Pokedex Access option
**** ⬜ Alternate wild lists to prevent version exclusives
**** ⬜ New events for accessing version-exclusive legendaries
**** ⬜ New events for accessing "choice" Pokemon e.g. all fossils, all starters
** ⬜ Difficulty and scaling options
*** ⬜ Cap levels by story/world progress
*** ⬜ Double Battles (enable/disable/force/allow-2v1)
*** ⬜ Toggle item use during battles
** ⬜ QOL options
*** ⬜ Start the game with Running Shoes already unlocked
**** ✅ Menu item
**** ⬜ Set FLAG_SYS_B_DASH when starting a new game
**** ✅ Littleroot Town dialogue/script changes
*** ⬜ Scale running speed on the overworld
*** ⬜ Toggle HM use without teaching moves
*** ⬜ Toggle reusable TMs
*** ⬜ Toggle re-battling missed legendaries
*** ⬜ Toggle event access (e.g. Eon Ticket)
*** ⬜ Toggle step counter instead of real-time clock
*** ⬜ Let's Play options
**** ⬜ Toggle infinite Rare Candies from game start
**** ⬜ Toggle infinite stat vitamins from game start
** ⬜ Randomizer options
*** ⬜ "Dynamic" versus "static" RNG
** ⬜ Nuzlocke options
*** ⬜ Slain Pokemon Behavior (Graveyard/Delete)
*** ⬜ Encounter Limit options
**** ⬜ Enable/Disable
**** ⬜ Gift Pokemon behavior (Free/Limits/Disable All Gifts)
**** ⬜ Shiny Exception (None/Replace Prior/Always Allow)
**** ⬜ Dupes Clause (Enable/Disable)
***** ⬜ Dupes Clause Shiny Exception (None/Replace Prior/Always Allow)
*** ⬜ Deactivation Condition (Never/After Champion/After Rayquaza/After All Legendaries)