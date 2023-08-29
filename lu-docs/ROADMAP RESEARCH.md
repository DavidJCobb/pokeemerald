
# Overworld pacing improvements

## Coalesce overworld and system messages

The scripts to handle picking up items on the overworld are in `data/scripts/obtain_item.inc`. The strings are at `data/text/obtain_item.inc`.


# Character customization

## Separate player pronouns from body, and add a neutral pronoun option

In theory, to show gendered dialogue, you'd use the `checkplayergender` script command (`ScrCmd_checkplayergender` in `src/scrcmd.c`) to show different strings. Surprisingly, however, I can't actually find any cases where the game does this outside of interactions with the rival, and cases where `checkplayergender` would be needed to properly manage Littleroot Town's state. (The `checkplayergender` command is indeed also used to properly set up the player-character and rival's sprites throughout the game, to manage the visibility of rival variants and other NPCs in Littleroot Town, and to manage which triggers activate in Littleroot Town's intro, so the script command and its behavior must be left as is.)

I can't find any cases where the player is directly referred to as "she" or "he," or via gendered terms like "lad" or "girl," though code for Japanese honorifics (*-kun* and *-chan*) exists (see below) and is tied to the player's sprite. Still, an engine-level pronoun option could be useful for story hacks, and we could just use a preprocessor macro to control whether the player is ever actually shown the option.

It's possible to build a proper pronoun system. It'd require leveraging and expanding the string placeholder system, and then rewriting every string that addresses the player or rival by pronoun (which, as established above, is none of them).

`StringExpandPlaceholders` in `gflib/string_util.c` takes a source buffer and destination buffer, and copies a string from the former to the latter. When it encounters a `PLACEHOLDER_BEGIN` character, it reads the next byte and passes that to `GetExpandedPlaceholder`; the expanded placeholder is then written into the destination buffer.

`GetExpandedPlaceholder` in `gflib/string_util.c` takes a single-byte placeholder code and returns a string pointer representing the expanded string; internally, it uses a mapping of placeholder codes to no-arguments functions. Placeholders exist for the player's name, and for a Japanese honorific suffix, as well as for various plot-critical NPCs.

The naive approach would be to define a slew of placeholders for the player and rivals' pronouns. There are five kinds of pronouns that we'd want, so we'd need to add ten placeholders:

* subject (he/she/they)
* object (him/her/them)
* possessive (his/hers/their)
* reflexive (himself/herself/themself)
* subject-is (he's/she's/they're)

A more robust approach would be to expand the placeholder system to allow multi-byte placeholder codes; for example, there could be a byte sequence representing `PLACEHOLDER PRONOUN PLAYER POSSESSIVE`, or a byte sequence representing `PLACEHOLDER PRONOUN RIVAL SUBJECT-IS`, or even a byte sequence representing `PLACEHOLDER PRONOUN MAXIE OBJECT`. Making this system generalizable to multiple NPCs could make it easier to do content revisions or total conversions (i.e. custom story hacks). This proposal means we need four bytes for every pronoun, but we avoid the need to duplicate strings en masse for pronouns.