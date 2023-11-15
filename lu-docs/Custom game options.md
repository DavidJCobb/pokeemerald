
# Custom Game Options

Inspired by Custom Games from the Halo series, a planned feature for this hack is Custom Game options that can be used to alter the game rules. These options can be used to set up challenge runs or to cheese and breeze your way through the game: customize your experience as you see fit.

## Available options

### Start w/ running shoes

Sets the `M_SYS_FLAG_DASH` flag on game start, which enables the ability to run. When your mom sees you off on your adventure after you receive the Pokedex, her dialogue omits all mention of the Running Shoes.


### Allow running indoors

Allows you to run inside of maps that have running disabled via their map headers, if and only if they're also flagged as indoor maps.


### Allow biking indoors

Allows you to use your bike inside of maps that have biking disabled via their map headers, if and only if they're also flagged as indoor maps.


### Battle options

#### Item use

Allows you to enable or disable the player's ability to use items while in battle. You can also set this to "No Backfield," which only allows the player to use items on their currently deployed Pokémon, the same limitation that NPC trainers have when using items; as a side effect, this inherently disallows reviving fallen Pokémon during a battle (since by definition they can't ever be on the field).

When this option is set to "Disabled," attempting to select the "Bag" option in battle will immediately display an error message, as is done in link battles. When this option is set to "No Backfield," backfield Pokémon display the text "NOT ABLE" when using items in battle, and they cannot be selected for item use.

As a side note, the "No Backfield" option was inspired by [a collaboration between Pokémon Challenges and SmallAnt](https://www.youtube.com/watch?v=_3VwGkml-nk) wherein the latter ran a Nuzlocke while the former gained control of all NPC trainers and tried to stop him. Unlike a typical NPC trainer, PChal could sacrifice weaker Pokémon to withdraw his heavy hitters and heal them in safety, and he was ruthless in taking advantage of this.

#### Scale accuracy

Allows you to scale the accuracy of your attacks, your NPC allies' attacks, and enemy attacks These options are percentages in the range [0%, 5000%]. Scaling is applied after all other accuracy calculations, and intentionally isn't accounted for by NPC AI.

#### Scale damage

Allows you to scale the damage that you deal, the damage that your NPC allies deal, and the damage that enemies deal. These options are percentages in the range [0%, 5000%]. Scaling is applied after all other damage calculations, and intentionally isn't accounted for by NPC AI.

#### Scale EXP gains

Scale the EXP awarded to the player's Pokémon with a multiplier in the range [0%, 5000%]. Different scaling options are available for non-traded and traded Pokémon, with each defaulting to the vanilla scales (100% and 150% respectively).

#### Scale payout after winning

Apply a percentage multiplier to the money the player earns after winning a battle. This only affects funds paid by the opposing Trainer, and not any other revenue e.g. Pay Day.

#### Modern money loss on defeat

In the classic Pokémon games, including Emerald, losing a battle would cost the player exactly half of all the money they were carrying. In FireRed, LeafGreen, and all newer games (from Gen IV onward), the player's money loss is calculated similarly to NPCs' money loss, while factoring in the player's badge count and the highest level amongst their party Pokémon. This setting can be used to enable the modern calculation.

### Pokemon catching options

#### Catch EXP

If enabled, your Pokemon will gain EXP if you catch a Wild Pokémon. The amount of EXP gained is the same as if the Wild Pokemon were defeated instead.

#### Catch rate increase

Apply a flat increase (defined as a percentage in the range [0%, 100%]) to all computed catch rates. Setting this value to 100%, for example, would increase your computed catch chance by 100%, guaranteeing successful captures every time.

#### Catch rate scale

Multiply the computed catch chance when throwing any Poké Ball. The multiplier is applied after all catch calculations are complete. The range is [0%, 5000%].

### Overworld poison damage

In the vanilla game, if a member of your party is poisoned, then they will take 1HP of damage after every four steps you take. (The step counter runs even when no one is poisoned.) Two custom game options exist for changing the damage interval (in steps) and the amount of damage dealt.

* The damage interval can be set to Disabled to turn overworld poison damage off, or can be set to any amount of steps in the range [1, 60].

* The damage dealt can be set to any value in the range [1, 2000].