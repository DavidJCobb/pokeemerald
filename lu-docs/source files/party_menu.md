
# `party_menu.h` and `party_menu.c`

These files handle the logic for displaying the Party Menu. You can access the menu in a variety of ways:

* The "Pokémon" option in the overworld pause menu.
* Using an item from the Bag &mdash; one which requires you to choose a Pokémon to use it on. The menu is used to make that selection.
* Giving mail or a hold item to a Pokémon. The menu is used to choose the Pokémon to give it to.
* Choosing to switch your currently deployed Pokémon out in battle. The menu is used to choose a Pokémon to replace it.
* Beginning a Multi Battle &mdash; that is, a battle with a partner wherein each of you contributes up to three Pokémon to the party. The menu is used to choose the three Pokémon you'll use.

The menu remembers where it was opened from and why it was opened, and these facts influence how it displays your party and what options it affords you. There are functions expoesd to the outside world to set this state properly, like `OpenPartyMenuInBattle` and `ChooseMonForInBattleItem`.


## Key source files

Some extra source files are used for this particular menu.

* `include/party_menu.h`
* `src/party_menu.c`
* `include/constants/party_menu.h`
   * This defines the constants for the party menu's type (i.e. where it was opened from), action (i.e. why it was opened), and layout, as well as a few other important enums.
* `src/data/party_menu.h`
   * This defines the constants for the party menu's graphics &mdash; the background layers, paint windows, bounding boxes for different data that can display on a party member ("info rects"), Pokémon sprite coordinates (for each menu layout and party slot), and so on.


## Key behaviors and functions

### Slots and ordering

Party menu state is stored in <code>gPartyMenu</code>. The current menu type (generally representing where the menu was opened from) is <code>gPartyMenu.menuType</code> and the current action is <code>gPartyMenu.action</code>; these are very important values to know about.

The party menu has six slots, naturally, numbered 0 through 6. They map directly to the order Pokémon are displayed in the menu *and* the order they're stored in `gPlayerParty`. But wait. When you're in a battle, the party menu displays your party in a battle-specific order, with the currently deployed Pokémon listed first (and second, for double battles). That order is affected by how you switch Pokémon in and out of the field, and it's also completely temporary, with your party being shown how you arranged it once the battle is over.

But wait. How can the party menu match both the display order within a battle, *and* the order of `gPlayerParty`? Well, the party menu reorders `gPlayerParty` when opened (`UpdatePartyToBattleOrder`) and when closed (`UpdatePartyToFieldOrder`). This means that during battles, `gPlayerParty[0]` is always your currently deployed Pokémon, and slot 0 is always the Pokémon you have on the field (as is slot 1 during double battles).

One last thing. During a multi battle &mdash; that is, a battle where you have an NPC ally, and each of you have contributed up to three Pokémon to a combined party &mdash; slots 1, 4, and 5 are the partner's Pokémon. The game determines that you're in a multi battle if the local function `IsMultiBattle` returns true; this function checks that the `BATTLE_TYPE_MULTI`, `BATTLE_TYPE_DOUBLE`, and `BATTLE_TYPE_TRAINER` bits in `gBattleTypeFlags` are all set, and that you're actually in battle (`gMain.inBattle`).


### `RenderPartyMenuBox` (or: how to display "ABLE", "NOT ABLE", and so on, on a party member)

This function is responsible for displaying a single party member, aside from its sprite. If a slot isn't empty, then this code runs:

```c
if (gPartyMenu.menuType == PARTY_MENU_TYPE_MOVE_RELEARNER)
    DisplayPartyPokemonDataForRelearner(slot);
else if (gPartyMenu.menuType == PARTY_MENU_TYPE_CONTEST)
    DisplayPartyPokemonDataForContest(slot);
else if (gPartyMenu.menuType == PARTY_MENU_TYPE_CHOOSE_HALF)
    DisplayPartyPokemonDataForChooseHalf(slot);
else if (gPartyMenu.menuType == PARTY_MENU_TYPE_MINIGAME)
    DisplayPartyPokemonDataForWirelessMinigame(slot);
else if (gPartyMenu.menuType == PARTY_MENU_TYPE_STORE_PYRAMID_HELD_ITEMS)
    DisplayPartyPokemonDataForBattlePyramidHeldItem(slot);
else if (!DisplayPartyPokemonDataForMoveTutorOrEvolutionItem(slot))
    DisplayPartyPokemonData(slot);
```

The `DisplayPartyPokemonData` function draws the full information for a party member: its name, gender, health, and so on. What are those other functions for?

When you try to use an evolution stone or teach a TM or HM to a Pokémon, you'll notice that a lot of its information *isn't* displayed; the game instead shows the words "ABLE" or "NOT ABLE". Similarly, when you need to choose three Pokémon to enter into certain battle types, you'll see "FIRST" or "SECOND" or "THIRD" on the Pokémon you've already selected. Those are handled by each of those "display party Pokémon data for..." functions. Let's look at two of them and see what we can learn.

#### `DisplayPartyPokemonDataForRelearner`

Here's the code for this function.

```c
static void DisplayPartyPokemonDataForRelearner(u8 slot)
{
    if (GetNumberOfRelearnableMoves(&gPlayerParty[slot]) == 0)
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NOT_ABLE_2);
    else
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_ABLE_2);
}
```

So this one is pretty simple: it's used for choosing a Pokémon to show to the Move Reminder NPC. We check the Pokémon that we want to display and see if it has any moves it can actually re-learn. If so, then we want to display "ABLE"; otherwise, we want to display "NOT ABLE". To display either piece of text, we use a helper function named `DisplayPartyPokemonDescriptionData`.

Yes, there are multiple constants for "ABLE" and "NOT ABLE". I suspect that each one probably translates to something slightly different in certain languages. The constants are defined in `src/data/party_menu.h`, as is the array (`sDescriptionStringTable`) that maps each one to a text string.

#### `DisplayPartyPokemonDataForChooseHalf`

This one's a little more involved. Here's the code:

```c
static void DisplayPartyPokemonDataForChooseHalf(u8 slot)
{
    u8 i;
    struct Pokemon *mon = &gPlayerParty[slot];
    u8 *order = gSelectedOrderFromParty;

    if (!GetBattleEntryEligibility(mon))
    {
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NOT_ABLE);
        return;
    }
    else
    {
        for (i = 0; i < GetMaxBattleEntries(); i++)
        {
            if (order[i] != 0 && (order[i] - 1) == slot)
            {
                DisplayPartyPokemonDescriptionData(slot, i + PARTYBOX_DESC_FIRST);
                return;
            }
        }
        DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_ABLE_3);
    }
}
```

We can see that first, we check whether the Pokémon in the current slot is even allowed to enter the battle that we're choosing Pokémon for (`GetBattleEntryEligibility`). If not, then we run the following command and exit; that command is a helper function that displays minimal Pokémon information along with a phrase like "FIRST" or "NOT ABLE":

```c
DisplayPartyPokemonDescriptionData(slot, PARTYBOX_DESC_NOT_ABLE);
```

If the Pokémon *is* eligible, however, then we check `gSelectedOrderFromParty`. Remember that for battle types where the player chooses three Pokémon, *they choose three Pokémon.* Like, all at once. They're not having to exit and re-enter the party menu each time. This means that the party menu needs to remember the *order* in which the player selected their Pokémon, and it does that using `gSelectedOrderFromParty`.

If the current Pokémon is in that list, then we display one of `PARTYBOX_DESC_FIRST`, `PARTYBOX_DESC_SECOND`, or `PARTYBOX_DESC_THIRD`, depending on its position in the list; the constants are defined sequentially so that `PARTYBOX_DESC_FIRST + 1` equals `PARTYBOX_DESC_SECOND` and so on.

If the current Pokémon is *not* in that list, then we display `PARTYBOX_DESC_ABLE_3`.


### `HandleChooseMonCancel` (or: how to handle backing out of the menu)

This function runs a switch-case on the current menu action. There are three cases of interest:

* `PARTY_ACTION_SEND_OUT` is the action used when you *must* send out a Pokémon during battle, e.g. because one was fainted and you need to send in a replacement. This case just plays the "failure" sound effect and exits.
* `PARTY_ACTION_SWITCH` is the action used when you're reordering your party, and `PARTY_ACTION_SOFTBOILED` is the action used when you're having one Pokémon use Softboiled to heal another while in the field. Both of these are "two-'mon actions," and so need special handling.
* The `default` case shows a yes/no confirmation box, sets up some variables to indicate your selection (i.e. "none"), and then queues the party menu to close.


### `HandleChooseMonSelection` (or: how to handle pressing A on a party member)

The function `HandleChooseMonSelection` *basically* handles what happens when you press A on a party member: do you get a menu with choices in it, or is some action (i.e. the one the party screen was opened for) performed on the selected Pokémon immediately? It also has to handle the case of the target Pokémon being ineligible for the chosen action. The function takes two arguments:

<dl>
   <dt><code>u8 taskId</code></dt>
      <dd>
         <p>The task ID being used to draw the menu.</p>
         <p>
            Most menu screens in Emerald will reset all tasks (which don't have a destructor system, so teardown code is not automatically run &mdash; yikes!), and then use tasks to manage their own behaviors and execution flow: generally a primary task for the menu flow, and secondary tasks for things like managing scroll indicators. However, since the Party Menu is meant to be invoked from a wide variety of different places, each of which need to maintain their own context and state even after the menu has closed, it can't do that. Instead, it co-opts a task supplied by its caller. This is the ID of that task.
         </p>
      </dd>
   <dt><code>s8* slotPtr</code></dt>
      <dd>
         <p>A <em>pointer</em> to the current slot number. I have no idea why they do it like this. They dereference it without ever checking whether it's <code>NULL</code>, so it's not meant to be, like, an Optional or something. In any case, <code>*slotPtr</code> is the slot number.</p>
      </dd>
</dl>

If pressing A on a Pokémon *shouldn't* bring up an action menu &mdash; if it should just perform an action immediately &mdash; then the general flow here is to check whether that action is allowed on the selected Pokémon. If so, perform it and queue the menu to close in whatever manner is appropriate. If the action isn't allowed, then just exit without doing anything.

But... what if we want to display a menu? Well, the `default` case, and the explicitly set behavior for `PARTY_ACTION_ABILITY_PREVENTS` and `PARTY_ACTION_SWITCHING`, is to display a selection window via `Task_TryCreateSelectionWindow`.


### Showing a choice menu when the player presses A on a Pokémon

Okay, this is a bit more involved.

First, we need to know that the `PARTY_ACTION_...` constants define reasons to open the menu &mdash; the thing we're going to use the party menu to do &mdash; but there's a similarly named set of constants with a very different meaning.

The `ACTIONS_...` constants, in `src/data/party_menu.h`, define a set of menu commands to be made available on a given Pokémon. Each one maps (via `sPartyMenuActions` in the same file) to a list of menu commands identified by `MENU_...` constants, like `MENU_SUMMARY` and `MENU_GIVE`. Those constants are in turn mapped to a UI string and a function pointer by the `sCursorOptions` array.

So how does the game decide which `ACTIONS_...` constant to make use of? Well, it does this in the `GetPartyMenuActionsType` function.

#### `GetPartyMenuActionsType`

This function takes a `struct Pokemon*` as an argument, and at the time that it runs, `gPartyMenu.slotId` is the slot that that Pokémon is in. The function returns an `ACTIONS_...` constant indicating which set of menu items should display.

The function does a switch-case on the current menu type. Based on this, it chooses which set of menu actions to expose for a given Pokémon. For example, if you're in the "choose three Pokémon for a battle" menu, it'll check the party slot's "entry status:"

* If the Pokémon in this slot is ineligible for the battle, we return `ACTIONS_SUMMARY_ONLY`: the player only has the "SUMMARY" menu item.
* If the Pokémon in this slot is eligible, we return `ACTIONS_ENTER`, which shows the "ENTER", "SUMMARY", and "CANCEL" menu items.
* If the Pokémon in this slot has already been chosen for battle, we return `ACTIONS_NO_ENTRY`, which returns the "NO ENTRY" (i.e. un-choose), "SUMMARY", and "CANCEL" menu items.

For certain menu types, we instead defer to other functions:

* For `PARTY_MENU_TYPE_IN_BATTLE`, we call `GetPartyMenuActionsTypeInBattle`. This function makes the following checks:

   * If the Pokémon has a "none" species or is an egg, return `ACTIONS_SUMMARY_ONLY`.
   * If the current action is `PARTY_ACTION_SEND_OUT`, then return `ACTIONS_SEND_OUT`, the list of menu items for when you *must* send a Pokémon into battle (e.g. to replace a fainted one).
   * If the current battle *is not* `BATTLE_TYPE_ARENA` (which limits you to 1v1 battles), then return `ACTIONS_SHIFT`, which allows the player to *voluntarily* switch a Pokémon into battle.
   
   You'll note that we don't actually run any checks related to item usage or similar, because using items in battle doesn't pop a choice menu. You select an item in the bag, and then you press A on a Pokémon (or press B to cancel) and the item is used immediately with no confirmation prompt.

* For `PARTY_MENU_TYPE_FIELD`, if we're not in the multi-partner room and the targt Pokémon is not an egg, we return `ACTIONS_NONE` here. Later on, that return value will be caught, and `SetPartyMonFieldSelectionActions` will be called, to populate the choice menu with overworld-specific options like field moves (with the list of moves and their behaviors defined in `src/data/party_menu.h`).