
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

#### Scale accuracy

Allows you to scale the accuracy of your attacks, your NPC allies' attacks, and enemy attacks These options are percentages in the range [0%, 5000%]. Scaling is applied after all other accuracy calculations, and intentionally isn't accounted for by NPC AI.

#### Scale damage

Allows you to scale the damage that you deal, the damage that your NPC allies deal, and the damage that enemies deal. These options are percentages in the range [0%, 5000%]. Scaling is applied after all other damage calculations, and intentionally isn't accounted for by NPC AI.

### Overworld poison damage

In the vanilla game, if a member of your party is poisoned, then they will take 1HP of damage after every four steps you take. (The step counter runs even when no one is poisoned.) Two custom game options exist for changing the damage interval (in steps) and the amount of damage dealt.

* The damage interval can be set to Disabled to turn overworld poison damage off, or can be set to any amount of steps in the range [1, 60].

* The damage dealt can be set to any value in the range [1, 2000].