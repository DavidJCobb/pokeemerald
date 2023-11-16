
# `Pokemon` data structure research

## Maintaining compatibility with vanilla games

Ideally, if we make changes to the Pokemon data structures, we should be able to maintain compatibility with vanilla games during trades, link battles, Recorded Battles, Record Mixing, and similar. This will require research into how the game performs link transfers for these situations.

It should be relatively trivial to write code that can take a `BoxPokemon*` or `Pokemon*` as input and fill a `VanillaCompatBoxPokemon` or `VanillaCompatPokemon`, remapping values as needed. Just gotta know when to do it.

Battles are especially troublesome, however, because they can transmit individual Pokemon attributes via `GetMonData` and `SetMonData`. The entire battle system is built in a way that unifies link battles, Battle Recordings, and similar, such that huge portions of the battle system run through those systems even when having a real-time single-player battle. Code for link contests also transmits some individual Pokemon properties using `GetMonData`, e.g. `Task_LinkContest_StartCommunicationEm` in `contest_link_util.c`.


## Potential changes

### Changes needed for new features

#### Hidden Abilities

Abilities were introduced in Gen III and at the time, a species could have up to two possible Abilities. Pokemon would track which ability they have via the `PokemonSubstruct3::abilityNum` field, which is a single bit.

Later games introduced Hidden Abilities, which were harder to obtain, meaning that a Pokemon species could have up to three possible Abilities. To implement this, `PokemonSubstruct3::abilityNum` would need to be extended to 2 bits, consuming 1 additional bit.


#### New Poke Balls

This would require increasing the bitcount used for `PokemonSubstruct3::pokeball`, the underlying storage for `MON_DATA_POKEBALL`. Additionally, all new Poke Ball item IDs must be contiguous with the existing IDs, *or* we must define a function that can remap item IDs from a global ID space to a Poke Ball-specific ID space (a similar function already exists and is used for displaying Poke Ball graphics, but the ID space used there has no "none" value; see `ItemIdToBallId` in `battle_anim.h`).

(Probably the easiest way to handle item IDs would be to just modify all the item constants so that the ones that come after Poke Balls are defined as `LAST_BALL + n` instead of `n`.)


### Optimizing the bitpacked format

As of the last time I updated this total, we can reclaim 39 bits (i.e. 4 bytes) across all four substructs by applying all of the changes below.

#### Reclaim 5 bits from `PokemonSubstruct0::species`

This value is stored as a `u16` and so supports all values in the range [0, 0xFFFF]. Per Bulbapedia, there are 1021 canon Pokemon species (including Ultra Beasts, and not including variants like Mega Pokemon) as of this writing, so we'd likely want to cap things at a maximum of 2047 to be future-compatible for a while longer. That means that a bitpacked Pokemon species ID would cost 11 bits to store, allowing us to reclaim 5 bits if we pack the species in the substruct.


#### Reclaim 7 bits from `PokemonSubstruct0::heldItem`

This value is stored as a `u16` and so supports all values in the range [0, 0xFFFF]. Currently, the game has 377 items defined, and our savegame codegen XML is written to cap item IDs at 511, consuming 9 bits. If we bitpack the held item ID here, then we save 7 bits.


#### Reclaim 20 bits (5 bits per) from `PokemonSubstruct1::moves[4]`

Move IDs are stored as `u16`s, and presently we do not bitpack them in the savegame.

Per Bulbapedia, across the entire series there are just over 900 ordinary moves, along with 35 Z-Moves, 33 G-Max moves, 18 Shadow moves, and 4 cut moves from Mystery Dungeon. If we cap move IDs at 2047 for extreme future-proofing, then we use 11 bits to store a Pokemon's move list, saving 5 bits per move (20 bits total).


#### Reclaim 3 bits from `PokemonSubstruct3::pokerus`

Pokerus is stored as a single byte, but this byte represents two four-bit values: the **strain** and the **days remaining**.

When a Pokemon is randomly infected, the days remaining are computed from the strain as `(strain % 4) + 1` a.k.a. `(strain & 0b11) + 1`. When a Pokemon is infected by a neighboring Pokemon in the party, the infectee copies the carrier's days remaining.

Once a Pokerus infection runs its course, the strain is remembered (which indicates the Pokemon's immunity and allows the EV boost to be permanent) but the days remaining become zero (which indicates that the Pokemon is no longer infected).

It's tempting to store the strain as a two-bit value, since only the two low bits determine the days remaining. However, a strain of 0 cannot occur naturally due to Emerald's RNG, and if a Pokemon with strain 0 were to wait out the days remaining, it would end up with a Pokerus byte of 0 (strain 0, days remaining 0) which would be impossible to distinguish from the Pokemon never having been infected before.

We can still reduce the bits consumed for Pokerus, however, by storing the strain as 3 bits (or as 2 bits with an additional "hasEverBeenInfected" bool) and the days remaining as 2 bits. This recovers 3 of the 8 bits currently used to track Pokerus.


#### Reclaim 4 bits by deleting `PokemonSubstruct3::unusedRibbons`

`PokemonSubstruct3::unusedRibbons` are discarded in Gen IV. As Gen III was never released on Virtual Console, there's no way to get Pokemon from Gen III into any Pokemon transfer service (e.g. Bank, HOME) without going through Gen IV first. This means that these bits will never be meaningful in a vanilla game, so we can and should repurpose them as we see fit within mods.


## Further research

* All contest ribbons earned by a Pokemon are stored using 3 bits (supporting the range [0, 7]). Contest ribbons are awarded by `GiveMonContestRibbon` (`contest_util.c`), and due to how the code is written, the highest possible ribbon it can ever award for any given category is (`CONTEST_RANK_MASTER + 1`) i.e. a value of 4. This value is equal to `CONTEST_RANK_LINK` (sentinel for remembering Link Contest winners) and `CONTEST_TYPE_NPC_MASTER` (unused?).

   It's unclear, then, whether we should allow this value to exceed `CONTEST_RANK_MASTER`. If we choose to cap it there, then we can reclaim one bit per contest category (five bits total), so long as we make sure to communicate correct data during link contests.
