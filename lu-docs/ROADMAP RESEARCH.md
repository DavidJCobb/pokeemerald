
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


# Cheat detection

There are a variety of GameShark and Action Replay codes available for Pokemon Emerald. However, using vanilla Emerald cheat codes on a decompilation-based ROM hack will inevitably lead to data corruption, as changes within code will cause cascading changes to the memory layout. This has led to a number of unfortunate situations where end users (who are by and large still used to ROM hacks based on binary editing) attempt to use vanilla cheat codes, break their playthroughs, and blame the hack and its author for the resulting chaos. In turn, some hack authors react to this by lashing out at anyone who uses, or wants to use, a cheat code; a number of ROM hacks have cheat detection scripts that are fairly vindictive, with effects like wiping the end user's playthrough entirely or disabling PokeCenter heals, often while browbeating them.

This situation benefits no one. It'd be nice if there were some way to detect when cheat codes are used and immediately display a brief explanatory screen, to warn the user about the unsafe situation they've put themselves in.


## Priorities for wording

* Briefly state that Pokemon Emerald cheat codes don't work with the ROM hack, and that continuing to play after enabling such a cheat code may lead to bugs.
* Encourage the user to disable cheats and revert to their last save or savestate prior to enabling them.
* Emphasize to the user that this is not anti-cheat
* Emphasize to the user that triggering the screen will not affect their playthrough (i.e. nothing later on, in the story or in gameplay, will react to them having attempted to cheat)
* Emphasize to the user that if they continue playing and save, the screen will not be displayed again.
* Consider having two button mappings (or a choice menu): one to dismiss the screen, and another to proceed to a specific explanation of why cheat codes don't work here.


## Possible implementation

It occurred to me that compilers and linkers must have some capability of placing specific variables at specific addresses, in order to grant access to hardware registers, mapped memory regions, and the like. It occurred to me: what if I find the RAM and ROM addresses targeted by common cheats, including the "master codes" used to disable Pokemon's ASLR implementation, and place constants in those spots? I could check their values every frame and if they change, I can pop the cheat detection screen.

I was... almost correct. Linkers like the one included with GCC will allow you to divide the address space into named "sections," and you can manually assign variables to these sections. However, [discontiguous sections risk wasting space](https://sourceware.org/binutils/docs/ld/Options.html#index-_002d_002denable_002dnon_002dcontiguous_002dregions) because the linker moves on if it encounters anything that won't fit in a section and won't come back to pack smaller items into the space it skipped; and if you try to overlap sections instead of using discontiguous sections, then nothing will be done to prevent address collisions. That is: you can't reserve a single small memory region and have all other data be automatically placed *around* it in an efficient manner. (And discontiguous sections [weren't even available until 2020](https://stackoverflow.com/questions/71621831/is-it-possible-to-auto-split-the-text-section-across-mulitple-memory-areas/77162469#77162469).)


## Cheat codes to target

Action Replay v3/v4 codes are encrypted using the algorithm described and presented [here](https://wunkolo.tumblr.com/post/144418662792) (corroborated by multiple other sources). This cannot be implemented in JavaScript; even if you use `Uint32Array`s to force all values to `uint32_t`s after every math operation, the language just isn't capable of anything that isn't floating-point, and you always get bad results.

Once you've decrypted the codes, they fall into the format listed [here](https://doc.kodewerx.org/hacking_gba.html#ardescribe). As far as I can tell, no one has ever bothered to make a (dis)assembler for these, so... I did.

The "master code" for Emerald doesn't do anything on its own except install a frame hook for the Action Replay to use, and allow it to auto-detect Pokemon Emerald based on a four-byte signature in the ROM header.

```
D8BAE4D9 4864DCE5 // master hook at 0x080005EC
A86CDBA5 19BA49B3 // auto-detect signature 0x0045455042
```

To kill ASLR, you need what's commonly (and incorrectly) known as [an "anti-DMA" code](https://projectpokemon.org/home/forums/topic/40558-gen-iii-master-codes-dma-disablers-all-languages/):

```
B2809E31 3CEF5320 //
1C7B3231 B494738C // write 0x2400 to 0x0800B5F7
```

Concerningly, many cheat code websites distribute codes that would necessarily *have* to tamper with ASLR-protected memory regions, without also including anti-ASLR codes. (Did the original Action Replay hardware automatically enable anti-ASLR codes for Pokemon games, once a master code allowed for auto-detection? I'm not confident that emulator cheats do.) Cheat code websites are run with a total lack of rigor; they are rife with technical ignorance, incomplete information, and outright plagiarism, with websites openly copying codes from one another without attribution (since few people have the know-how to make their own; cheat code formats were intentionally obfuscated by the long-dead companies that designed them) and often without the original instructions attached. In many cases, websites mix and match different, mutually incompatible cheat code formats without specifying which format any individual code is in. While this *does* mean that online cheat code websites are a forsaken wasteland, it also means that there aren't actually that *many* codes circulating out there, and so there won't be that many addresses that users may attempt to hot-patch using cheat devices.

So, here are some addresses that get patched for various means. Note that I haven't tested any of these codes to ensure that they work as claimed.

### Anti-ASLR ("anti-DMA")
```
B2809E31 3CEF5320 //
1C7B3231 B494738C // write 0x2400 to 0x0800B5F7
```

### Force Wild Pokemon gender (female)
```
7F06853C D823D089 // 
0A694B5A 43A6964F // write 0x2100 to 0x080057BC
```

### Force Wild Pokemon gender (male)
```
7F06853C D823D089 // 
7980105E FC3721D0 // write 0x21FF to 0x080057BC
```

### Force Wild Pokemon level
Each level is one code.
```
65B97174 4A8FBA5C // write 0x01 to 0x03007CF8 // level 1
C1AEDD79 5939EF0C // write 0x02 to 0x03007CF8 // level 2
68770050 678B8139 // write 0x64 to 0x03007CF8 // level 100
```

### Force Wild Pokemon nature
This code forces a Hardy nature:
```
6658C989 89518A0F // 
217A1A8B E666BC3F // write 0x2500 to 0x08003F57
```

The second line (which specifies the value to actually write) can be varied for different natures. For example, `CD72C719 F2831689` will write 0x2501, producing a Lonely nature.

### Force Wild Pokemon shiny state
```
F3A9A86D 4E2629B4 // 
18452A7D DDE55BCC // write 0x6039 to 0x080057D3
```

### Force Wild Pokemon species
This code forces CHarizard:
```
D8BAE4D9 4864DCE5 // master code line 1 (labeled as such)
A86CDBA5 19BA49B3 // master code line 2 (labeled as such)
B749822B CE9BFAC1 // a SECOND master code, NOT lableed as such, line 1
A86CDBA5 19BA49B3 // a SECOND master code, NOT lableed as such, line 2
A72AB10A 7F70076E // write 0x0006 to 0x03007CF6
```
The last line controls the species; for example, `28D1617C 346C04D9` writes 0x00C8 to the target address, producing an adorable Misdreavus.

### Light up dark areas
```
0C7BD341 E9775222 // write 0x00 to 0x02009B3D
```

### Infinite Master Balls in PC
The PC inventory is in `SaveBlock1` and is therefore ASLR-protected. You need an anti-ASLR code, not just a master code, for this to work safely.
```
128898B6 EDA43037 // write 0x0001 to 0x02005EE0
```

This variation writes Rare Candies (0x0044):
```
BFF956FA 2F9EC50D // write 0x0044 to 0x02005EE0
```

### Infinite money (A)
```
A57E2EDE A5AFF3E4 // 
1C7B3231 B494738C // write 0x2400 to ox0800B5F5
C051CCF6 975E8DA1 // write 999999 to 0x02005E90
```
Sometimes, the second line is posted on its own.

### Infinite move PP
```
BBB3B3B0 7AF06B2A // write 0x63636363 to 0x020040A8
```

### Infinite rare candies
```
BFF956FA 2F9EC50D // write 0x0044 to 0x02005EE0
```

### No random battles
```
B505DB41 6E39EA4E // write 0x0000 to 0x020075D4
```

### Override next door
Each destination is one code.
```
6266061B C8C9D80F // Write 0x0001 to 0x020022E4 // Player's house
6178C413 3E2FF736 // Write 0x0010 to 0x020022E4 // Elite Four Sidney's room
41509519 56FA6E47 // Write 0x0111 to 0x020022E4 // Route 104 flower shop
A6DF8006 B219CFD6 // Write 0x0112 to 0x020022E4 // Route 111 rest stop
B49EB935 5B46BF8B // Write 0x0200 to 0x020022E4 // Mauville City
93DF5785 8CD8CE27 // Write 0x0300 to 0x020022E4 // Rustboro City
B6D1DB88 5987C390 // Write 0x0400 to 0x020022E4 // Fortree City
16830A6F EEFE51C8 // Write 0x0800 to 0x020022E4 // Ever Grande City
F0EC26B8 31EC5657 // Write 0x0A1A to 0x020022E4 // Southern Island
```
...You get the idea.

### PokeMart override
Master Balls:
```
958D8046 A7151D70 // 
8BB602F7 8CEB681A // write 0x2101 to 0x0800FFB0
```

### Press R to increase XP when fighting wild Pokemon
```
43D8AC45 0D3B349A // if 0x01 == *(uint8_t*)0x030022E9 then run next line
3DC869E3 D39C09B2 // write 0x0500 to 0x020041F0
```

### Steal trainers' Pokemon
```
B6C5368A 08BE8FF4 // if 0x00FF == *(uint16_t*)0x04000130, then run next line
B8D95CFE 06ED6EA1 // write 0x04 to 0x02002FEC
E151C402 8A229A83 // 
8E883EFF 92E9660D // write 0x2000 to 0x08003DF2
```

### Walk through walls (A)
This is actually the same code twice. The decrypted form of the latter uses 0xC0DE0ACE to fill some padding in the code format.
```
7881A409 E2026E0C //
8E883EFF 92E9660D // write 0x2000 to 0x08009603
7881A409 E2026E0C //
C56CFACA DC167904 // write 0x2000 to 0x08009603
```