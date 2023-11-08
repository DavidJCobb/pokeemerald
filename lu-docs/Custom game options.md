
# Custom Game Options

## Available options

### Start w/ running shoes

Sets the `M_SYS_FLAG_DASH` flag on game start, which enables the ability to run. When your mom sees you off on your adventure after you receive the Pokedex, her dialogue omits all mention of the Running Shoes.


### Allow running indoors

Allows you to run inside of maps that have running disabled via their map headers, if and only if they're also flagged as indoor maps.


### Allow biking indoors

Allows you to use your bike inside of maps that have biking disabled via their map headers, if and only if they're also flagged as indoor maps.


### Battle options

#### Scale accuracy

Allows you to scale the accuracy of your attacks, your NPC allies' attacks, and enemy attacks These options are percentages in the range [0%, 5000%]. Scaling is applied after all other accuracy calculations, and is not accounted for by NPC AI.

#### Scale damage

Allows you to scale the damage that you deal, the damage that your NPC allies deal, and the damage that enemies deal. These options are percentages in the range [0%, 5000%]. Scaling is applied after all other damage calculations, and is not accounted for by NPC AI.

### Overworld poison damage

In the vanilla game, if a member of your party is poisoned, then they will take 1HP of damage after every four steps you take. The step count is relative to when you started your playthrough, though it is sometimes cleared early during play. Two custom game options exist for changing the damage interval (in steps) and the amount of damage dealt.

* The damage interval can be set to Disabled to turn overworld poison damage off, or can be set to any amount of steps in the range [1, 60].

* The damage dealt can be set to any value in the range [1, 2000].