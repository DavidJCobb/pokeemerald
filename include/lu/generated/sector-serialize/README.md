# Codegen for bitpacked savedata
This report describes the last successful attempt at code generation.

In vanilla `pokeemerald`, the `SaveBlock1` (world state) and `SaveBlock2` (character state) structs consume 99% of the space allotted to them in flash memory (savedata). This is because they are blindly `memcpy`'d from RAM. A bitpacked format would consume substantially less space; however, maintaining the code to bitpack these structs would be prohibitively difficult by hand because savedata is split into ~4KiB strips ("sectors"). Instead, we use custom code generation to produce the serialization code, minding sector boundaries.

## Overall stats
53408 bytes in RAM / 55552 bytes available across 14 sectors (96% space usage)  
406820 packed bits = 50853 packed bytes (91% space usage)  


## Struct stats
| Name | Bytes in RAM | Packed bits | Packed bytes | Savings |
| - | -: | -: | -: | -: |
| SaveBlock2 | 3912 | 27675 |3460 | 452 (11%) |
| SaveBlock1 | 15752 | 111433 |13930 | 1822 (11%) |
| PokemonStorage | 33744 | 267712 |33464 | 280 (0%) |
| **Total** | 53408 | 406820 |50853 | 2555 (4%) |


## Sector group: CharacterData
**Status:** Successfully packed into 1 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 27675 | 31744 | 87% |
| Total | 27675 | 31744 | 87% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock2`
## Sector group: WorldData
**Status:** Successfully packed into 4 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 31724 | 31744 | 99% |
| 1 | 31729 | 31744 | 99% |
| 2 | 31724 | 31744 | 99% |
| 3 | 16256 | 31744 | 51% |
| Total | 111433 | 126976 | 87% |

### Member info
* Last field serialized to sector 0: `p_SaveBlock1->objectEventTemplates[62].trainerType`
* Last field serialized to sector 1: `p_SaveBlock1->secretBases[14].decorationPositions[1]`
* Last field serialized to sector 2: `p_SaveBlock1->mysteryGift.cardCrc`
* Last field serialized to sector 3: `p_SaveBlock1->waldaPhrase`
## Sector group: PokemonStorage
**Status:** Successfully packed into 9 sectors.


| Sector # | Bits used | Bits available | % |
| - | -: | -: |
| 0 | 31722 | 31744 | 99% |
| 1 | 31718 | 31744 | 99% |
| 2 | 31718 | 31744 | 99% |
| 3 | 31718 | 31744 | 99% |
| 4 | 31718 | 31744 | 99% |
| 5 | 31718 | 31744 | 99% |
| 6 | 31718 | 31744 | 99% |
| 7 | 31718 | 31744 | 99% |
| 8 | 13964 | 31744 | 43% |
| Total | 267712 | 285696 | 93% |

### Member info
* Last field serialized to sector 0: `p_PokemonStorage->boxes[1][19].secure.raw[10]`
* Last field serialized to sector 1: `p_PokemonStorage->boxes[3][9].secure.raw[9]`
* Last field serialized to sector 2: `p_PokemonStorage->boxes[4][29].secure.raw[8]`
* Last field serialized to sector 3: `p_PokemonStorage->boxes[6][19].secure.raw[7]`
* Last field serialized to sector 4: `p_PokemonStorage->boxes[8][9].secure.raw[6]`
* Last field serialized to sector 5: `p_PokemonStorage->boxes[9][29].secure.raw[5]`
* Last field serialized to sector 6: `p_PokemonStorage->boxes[11][19].secure.raw[4]`
* Last field serialized to sector 7: `p_PokemonStorage->boxes[13][9].secure.raw[3]`
* Last field serialized to sector 8: `p_PokemonStorage->boxWallpapers[*]`


## Every struct
The "Savings" columns are measured in bytes. The percentage in the "Savings per" column indicates by how much the struct itself has been made smaller thanks to bitpacking. The percentage in the "Savings total" column indicates by how much the total size of all serialized data has been made smaller as a result of bitpacking that row's specific struct type.

| Name | Bytes in RAM | Packed bits | Packed bytes | Savings per | Count | Savings total |
| - | -: | -: | -: | -: | -: | -: |
| Apprentice | 68 | 494 | 62 | 6 (8%) | 4 | 24 (0%) |
| ApprenticeMon | 12 | 84 | 11 | 1 (8%) | 12 | 12 (0%) |
| ApprenticeQuestion | 4 | 24 | 3 | 1 (25%) | 9 | 9 (0%) |
| BattleDomeTrainer | 4 | 16 | 2 | 2 (50%) | 16 | 32 (0%) |
| BattleFrontier | 2300 | 16558 | 2070 | 230 (10%) | 1 | 230 (3%) |
| BattleTowerEReaderTrainer | 188 | 1430 | 179 | 9 (4%) | 1 | 9 (0%) |
| BattleTowerInterview | 24 | 161 | 21 | 3 (12%) | 1 | 3 (0%) |
| BattleTowerPokemon | 44 | 330 | 42 | 2 (4%) | 27 | 54 (2%) |
| Berry2 | 28 | 208 | 26 | 2 (7%) | 1 | 2 (0%) |
| BerryCrush | 16 | 128 | 16 | 0 (0%) | 1 | 0 (0%) |
| BerryPickingResults | 16 | 128 | 16 | 0 (0%) | 1 | 0 (0%) |
| BerryTree | 8 | 48 | 6 | 2 (25%) | 128 | 256 (1%) |
| BoxPokemon | 80 | 635 | 80 | 0 (0%) | 428 | 0 (64%) |
| ContestWinner | 32 | 227 | 29 | 3 (9%) | 13 | 39 (0%) |
| Coords16 | 4 | 32 | 4 | 0 (0%) | 49 | 0 (0%) |
| DayCare | 288 | 2162 | 271 | 17 (5%) | 1 | 17 (0%) |
| DaycareMail | 56 | 394 | 50 | 6 (10%) | 2 | 12 (0%) |
| DaycareMon | 140 | 1061 | 133 | 7 (5%) | 2 | 14 (0%) |
| DewfordTrend | 8 | 63 | 8 | 0 (0%) | 5 | 0 (0%) |
| DomeMonData | 16 | 120 | 15 | 1 (6%) | 3 | 3 (0%) |
| EmeraldBattleTowerRecord | 236 | 1763 | 221 | 15 (6%) | 6 | 90 (2%) |
| EnigmaBerry | 52 | 400 | 50 | 2 (3%) | 1 | 2 (0%) |
| ExternalEventData | 20 | 160 | 20 | 0 (0%) | 1 | 0 (0%) |
| ExternalEventFlags | 21 | 163 | 21 | 0 (0%) | 1 | 0 (0%) |
| GabbyAndTyData | 12 | 86 | 11 | 1 (8%) | 1 | 1 (0%) |
| ItemSlot | 4 | 19 | 3 | 1 (25%) | 236 | 236 (1%) |
| LinkBattleRecord | 16 | 120 | 15 | 1 (6%) | 5 | 5 (0%) |
| LinkBattleRecords | 88 | 615 | 77 | 11 (12%) | 1 | 11 (0%) |
| Mail | 36 | 252 | 32 | 4 (11%) | 18 | 72 (1%) |
| MysteryGiftSave | 876 | 6966 | 871 | 5 (0%) | 1 | 5 (1%) |
| ObjectEvent | 36 | 276 | 35 | 1 (2%) | 16 | 16 (1%) |
| ObjectEventTemplate | 24 | 160 | 20 | 4 (16%) | 64 | 256 (2%) |
| PlayersApprentice | 44 | 261 | 33 | 11 (25%) | 1 | 11 (0%) |
| PokeNews | 4 | 32 | 4 | 0 (0%) | 16 | 0 (0%) |
| Pokeblock | 8 | 56 | 7 | 1 (12%) | 40 | 40 (0%) |
| Pokedex | 120 | 960 | 120 | 0 (0%) | 1 | 0 (0%) |
| Pokemon | 100 | 794 | 100 | 0 (0%) | 6 | 0 (1%) |
| PokemonJumpRecords | 16 | 112 | 14 | 2 (12%) | 1 | 2 (0%) |
| PokemonStorage | 33744 | 267712 | 33464 | 280 (0%) | 1 | 280 (62%) |
| PyramidBag | 60 | 320 | 40 | 20 (33%) | 1 | 20 (0%) |
| RamScript | 1004 | 8024 | 1003 | 1 (0%) | 1 | 1 (1%) |
| RamScriptData | 1000 | 7992 | 999 | 1 (0%) | 1 | 1 (1%) |
| RankingHall1P | 16 | 107 | 14 | 2 (12%) | 54 | 108 (1%) |
| RankingHall2P | 28 | 195 | 25 | 3 (10%) | 6 | 18 (0%) |
| RecordMixingGift | 16 | 121 | 16 | 0 (0%) | 1 | 0 (0%) |
| RecordMixingGiftData | 12 | 89 | 12 | 0 (0%) | 1 | 0 (0%) |
| RentalMon | 12 | 59 | 8 | 4 (33%) | 6 | 24 (0%) |
| Roamer | 28 | 147 | 19 | 9 (32%) | 1 | 9 (0%) |
| SaveBlock1 | 15752 | 111433 | 13930 | 1822 (11%) | 1 | 1822 (26%) |
| SaveBlock2 | 3912 | 27675 | 3460 | 452 (11%) | 1 | 452 (6%) |
| SecretBase | 160 | 1181 | 148 | 12 (7%) | 20 | 240 (5%) |
| SecretBaseParty | 108 | 786 | 99 | 9 (8%) | 20 | 180 (3%) |
| Time | 8 | 36 | 5 | 3 (37%) | 2 | 6 (0%) |
| TrainerHillSave | 12 | 88 | 11 | 1 (8%) | 1 | 1 (0%) |
| TrainerNameRecord | 12 | 88 | 11 | 1 (8%) | 20 | 20 (0%) |
| WaldaPhrase | 24 | 177 | 23 | 1 (4%) | 1 | 1 (0%) |
| WarpData | 8 | 56 | 7 | 1 (12%) | 5 | 5 (0%) |
| WonderCard | 332 | 2635 | 330 | 2 (0%) | 1 | 2 (0%) |
| WonderCardMetadata | 36 | 283 | 36 | 0 (0%) | 1 | 0 (0%) |
| WonderNews | 444 | 3552 | 444 | 0 (0%) | 1 | 0 (0%) |
| WonderNewsMetadata | 4 | 16 | 2 | 2 (50%) | 1 | 2 (0%) |



## Every heritable data definition

| Name | Uses in CharacterData | Uses in WorldData | Uses in PokemonStorage | Total uses |
| - | -:  | -:  | -:  | -: |
| $player-name | 80 | 86 | 420 | 586 |
| $game-language | 71 | 33 | 420 | 524 |
| $item-quantity | 1 | 0 | 0 | 1 |
| $global-item-id | 41 | 276 | 0 | 317 |
| $pokemon-move-id | 42 | 22 | 0 | 64 |
| $pokemon-nickname | 28 | 23 | 420 | 471 |
| $pokemon-species-id | 49 | 57 | 0 | 106 |
| $easy-chat-word | 25 | 28 | 0 | 53 |

