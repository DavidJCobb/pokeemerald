
# Script commands


## Terminology

<dl>
   <dt>script context</dt>
      <dd>An internal data structure (<code>ScriptContext</code>) used to keep track of script execution. This stores the memory address of the current script instruction, the current call stack, and more.</dd>
   <dt>script context userdata</dt>
      <dd>A set of four 32-bit (four-byte) integers present on the script context, which are available for use by specific script commands.</dd>
</dl>


## Common operand types

<dl>
   <dt>s8</dt>
      <dd>An 8-bit (one-byte) signed integer.</dd>
   <dt>u8</dt>
      <dd>An 8-bit (one-byte) unsigned integer.</dd>
   <dt>u16</dt>
      <dd>A 16-bit (two-byte) unsigned integer.</dd>
   <dt>u32</dt>
      <dd>A 32-bit (four-byte) unsigned integer.</dd>
   <dt>pointer</dt>
      <dd>A 32-bit (four-byte) memory address encoded in little-endian. Typically, this will be a ROM address.</dd>
   <dt>flag</dt>
      <dd>
         <p>The 16-bit (two-byte) ID of an event script flag.</p>
         <p>ID 0 is considered invalid.</p>
         <p>IDs in the range [0x0001, 0x001F] are temporary flags and will be cleared every time a map is loaded.</p>
         <p>IDs in the range [0x0020, 0x04FF] are persistent script flags.</p>
         <p>IDs in the range [0x0500, 0x085F] are trainer flags. Increasing <code>MAX_TRAINERS_COUNT</code> will extend this range and displace later flags.</p>
         <p>IDs in the range [0x0860, 0x0866] are system flags.</p>
         <p>IDs in the range [0x0867, 0x086E] track Gym Badge acquisition.</p>
         <p>IDs in the range [0x086F, 0x087E] track whether the player has visited towns and cities.</p>
         <p>IDs in the range [0x087F, 0x091F] are various other persistent or temporary flags.</p>
         <p>IDs in the range [0x0920, 0x0960] are daily flags and will be cleared once per day.</p>
         <p>IDs in the range [0x4000, 0x407F] are special flags and are not written to the savegame.</p>
      </dd>
   <dt>variable</dt>
      <dd>
         <p>The 16-bit (two-byte) ID of an event script variable.</p>
         <p>IDs in the range [0x4000, 0x400F] are temporary variables and will be cleared every time a map is loaded.</p>
         <p>IDs in the range [0x4010, 0x40FF] are persistent script variables.</p>
         <p>IDs in the range [0x4100, 0x7FFF] are <strong>invalid</strong> and will index out-of-bounds into other savedata.</p>
         <p>IDs in the range [0x8000, 0x8015] are special variables that various commands or standard scripts are hardcoded to use.</p>
      </dd>
   <dt>variable-or-u16</dt>
      <dd>A 16-bit value. Values below 0x4000 are treated as integer constants; all other values are treated as variable IDs.</dd>
   <dt>std-script-index</dt>
      <dd>An 8-bit (one-byte) index into a list of "standard scripts." If you specify an out-of-bounds index, then commands taking this operand will generally fail.</dd>
   <dt>userdata-index</dt>
      <dd>An 8-bit (one-byte) integer, referring to one of the four userdata integers on the script context.</dd>
</dl>

## All commands

### nop
**Opcode:** 0x00 and others  
**Syntax:** `nop`

This command reads no arguments and does nothing.


### end
**Opcode:** 0x02  
**Syntax:** `end`

Immediately stops execution of the current script.

Under the hood, this function just calls `StopScript(ctx)`.

### return

**Opcode:** 0x03  
**Syntax:** `return`

Jump to the script data at the topmost *return address* (see `call`).


### call
**Opcode:** 0x04  
**Syntax:** `call <pointer>`

Temporarily jump to the script data at a given memory address, and continue executing from there. When a subsequent `return` instruction is encountered, we will then jump to the instruction after this `call` (the <dfn>return address</dfn>).

The game remembers a return address for each call it encounters, and every call should generally lead to code that ends with a matching return. Return addresses are stored in a "stack," being "pushed" onto the top of the stack with each `call` and "popped" off the top of the stack with each `return`. The stack can store up to twenty return addresses; past that, calls will fail.


### goto
**Opcode:** 0x05  
**Syntax:** `goto <pointer>`

Jump to the script data at a given memory address, and continue executing from there.


### goto_if
**Opcode:** 0x06


### call_if
**Opcode:** 0x07


### gotostd
**Opcode:** 0x08  
**Syntax:** `gotostd <std-script-index>`

Jump to the specified standard script and execute it.


### callstd
**Opcode:** 0x09  
**Syntax:** `callstd <std-script-index>`

Call (`call`) the specified standard script.


### gotostd_if
**Opcode:** 0x0A


### callstd_if
**Opcode:** 0x0B


### returnram
**Opcode:** 0x0C

### endram
**Opcode:** 0x0D

### setmysteryeventstatus
**Opcode:** 0x0E  
**Syntax:** `setmysteryeventstatus <u8 index>`

Set the Mystery Event script status to the given `index`.

### loadword
**Opcode:** 0x0F

### loadbyte
**Opcode:** 0x10  
**Syntax:** `loadbyte <userdata-index> <u8 value>`

Overwrites one of the userdata integers on the script context with `value`.


### setptr
**Opcode:** 0x11  
**Syntax:** `setptr <u8 value> <pointer>`

Sets the byte at `pointer` to `value`.


### loadbytefromptr
**Opcode:** 0x12  
**Syntax:** `loadbytefromptr <userdata-index> <pointer>`

Loads the byte at `pointer`, and uses it to overwrite one of the userdata integers on the script context.


### setptrbyte
**Opcode:** 0x13  
**Syntax:** `setptrbyte <userdata-index> <pointer>`

Sets the byte at `pointer` to one of the script context userdata integers, truncating the integer to a byte when writing.


### copylocal
**Opcode:** 0x14  
**Syntax:** `copylocal <userdata-index destination> <userdata-index source>`

Overwrites one of the script context userdata integers (the "destination") with a copy of another (the "source").


### copybyte
**Opcode:** 0x15  
**Syntax:** `copylocal <pointer destination> <pointer source>`

Overwrites the 8-bit value at the destination pointer with the 8-bit value at the source pointer.


### setvar
**Opcode:** 0x16  
**Syntax:** `setvar <variable> <u16 value>`

Sets the specified variable to `value`.


### addvar
**Opcode:** 0x17  
**Syntax:** `addvar <variable> <u16 value>`

Increases the specified variable's value by `value`. Some vanilla scripts take advantage of integer overflow to decrease variable values using this command.


### subvar
**Opcode:** 0x18  
**Syntax:** `addvar <variable> <variable-or-value>`

Decreases the specified variable's value by the given operand.


### copyvar
**Opcode:** 0x19  
**Syntax:** `copyvar <variable destination> <variable source>`

Sets the destination variable to the value of the source variable.


### setorcopyvar
**Opcode:** 0x1A  
**Syntax:** `setorcopyvar <variable destination> <variable-or-u16 source>`

Sets the destination variable to the source. If the source is a variable, its value is used; otherwise, the source is an integer constant, and is the value that gets used.


### compare_local_to_local
**Opcode:** 0x1B  
**Syntax:** `compare_local_to_local <userdata-index> <userdata-index>`

Compares the values of the two specified script context userdata integers. The result is set to: 0 if the former is less than the latter; 1 if they are equal; or 2 otherwise.


### compare_local_to_value
**Opcode:** 0x1C  
**Syntax:** `compare_local_to_local <userdata-index> <u8>`

Compares the value of the specified script context userdata integer to the given constant. The result is set to: 0 if the former is less than the latter; 1 if they are equal; or 2 otherwise.



### compare_local_to_ptr
**Opcode:** 0x1D  
**Syntax:** `compare_local_to_local <userdata-index> <pointer>`

Compares the value of the specified script context userdata integer to the unsigned byte located at the given pointer. The result is set to: 0 if the former is less than the latter; 1 if they are equal; or 2 otherwise.


### compare_ptr_to_local
**Opcode:** 0x1E

### compare_ptr_to_value
**Opcode:** 0x1F

### compare_ptr_to_ptr
**Opcode:** 0x20

### compare_var_to_value
**Opcode:** 0x21  
**Syntax:** `compare_var_to_value <variable> <u16>`

Compares the value of the given variable to the given constant integer. The result is set to: 0 if the former is less than the latter; 1 if they are equal; or 2 otherwise.


### compare_var_to_var
**Opcode:** 0x22  
**Syntax:** `compare_var_to_value <variable> <variable>`

Compares the values of the two given variables. The result is set to: 0 if the former is less than the latter; 1 if they are equal; or 2 otherwise.


### callnative
**Opcode:** 0x23

### gotonative
**Opcode:** 0x24

### special
**Opcode:** 0x25

### specialvar
**Opcode:** 0x26

### waitstate
**Opcode:** 0x27

### delay
**Opcode:** 0x28  
**Syntax:** `delay <u16 count>`

Pauses script execution for <var>count</var> frames.


### setflag
**Opcode:** 0x29  
**Syntax:** `setflag <flag>`

Sets the specified flag to 1.


### clearflag
**Opcode:** 0x2A  
**Syntax:** `clearflag <flag>`

Sets the specified flag to 0.


### checkflag
**Opcode:** 0x2B


### initclock
**Opcode:** 0x2C  
**Syntax:** `initclock <variable-or-value hour> <variable-or-value minute>`

Calls `RtcInitLocalTimeOffset(hour, minute)`.


### dotimebasedevents
**Opcode:** 0x2D
**Syntax:** `dotimebasedevents`

Calls `DoTimeBasedEvents()`.


### gettime
**Opcode:** 0x2E
**Syntax:** `gettime`

Calls `RtcCalcLocalTime()`, and then sets the following variables to reflect the real-time clock's current value:

| Var | Value |
| - | - |
| 0x8000 | Hours |
| 0x8001 | Minutes |
| 0x8002 | Seconds |


### playse
**Opcode:** 0x2F

### waitse
**Opcode:** 0x30

### playfanfare
**Opcode:** 0x31

### waitfanfare
**Opcode:** 0x32

### playbgm
**Opcode:** 0x33

### savebgm
**Opcode:** 0x34

### fadedefaultbgm
**Opcode:** 0x35

### fadenewbgm
**Opcode:** 0x36

### fadeoutbgm
**Opcode:** 0x37

### fadeinbgm
**Opcode:** 0x38

### warp
**Opcode:** 0x39  
**Syntax:** `warp <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Teleports the player to the specified map and position by calling `SetWarpDestination`, `DoWarp`, and `ResetInitialPlayerAvatarState`. All values will be interpreted as signed integers.

The game will use the <var>warp-id</var> to position the player at a warp point on the target map if the ID is valid. If the ID is invalid, the game will use the <var>x</var> and <var>y</var> coordinates to position the player if they are valid. If none of those operands are valid, the game will position the player at the center of the map.

A valid warp ID is non-negative and does not exceed the target map's max warp ID. Prefer `WARP_ID_NONE` as an invalid ID if you wish to forego using a warp point on the map.

Valid <var>x</var> and <var>y</var> coordinates are non-negative when interpreted as signed two-byte integers. The game does not check whether they would place the player out of bounds to the south or east.


### warpsilent
**Opcode:** 0x3A  
**Syntax:** `warpsilent <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Similar to `warp`, but under the hood, `DoDiveWarp` is used instead of `DoWarp`.

### warpdoor
**Opcode:** 0x3B  
**Syntax:** `warpdoor <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Similar to `warp`, but under the hood, `DoDoorWarp` is used instead of `DoWarp`.


### warphole
**Opcode:** 0x3C  
**Syntax:** `warphole <s8 map-group> <s8 map-num>`

If the map group is `MAP_GROUP(UNDEFINED)` and the map number is `MAP_NUM(UNDEFINED)`, then this function selects a destination to warp the player to via `SetWarpDestinationToFixedHoleWarp`. Otherwise, it warps the player to the specified map.

The player's coordinates are computed via `PlayerGetDestCoords`, and will be the same as their coordinates on the current map.

The `SetWarpDestinationToFixedHoleWarp` function (in `src/overworld.c`) uses: the map number and group from `sFixedHoleWarp` if all fields in that struct are currently defined; or the map number and group, and X- and Y-coordinates, from `gLastUsedWarp` otherwise. You can set `sFixedHoleWarp` using `setholewarp`.

The `DoFallWarp` function is used to actually perform the warp.


### warpteleport
**Opcode:** 0x3D  
**Syntax:** `warpteleport <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Similar to `warp`, but under the hood, `DoTeleportTileWarp` is used instead of `DoWarp`.


### setwarp
**Opcode:** 0x3E  
**Syntax:** `setwarp <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Sets the next warp destination (`sWarpDestination`), but doesn't actually send the player anywhere.


### setdynamicwarp
**Opcode:** 0x3F  
**Syntax:** `setdynamicwarp <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Sets the next "dynamic" warp destination (`gSaveBlock1Ptr->dynamicWarp`), but doesn't actually send the player anywhere.


### setdivewarp
**Opcode:** 0x40  
**Syntax:** `setdivewarp <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Sets `sFixedDiveWarp`, but doesn't actually send the player anywhere. This destination appears to be used if the game is induced to attempt a dive warp on a map that doesn't have a map connection in an appropriate direction (i.e. `CONNECTION_EMERGE` when trying to emerge or `CONNECTION_DIVE` when trying to dive).


### setholewarp
**Opcode:** 0x41  
**Syntax:** `setholewarp <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Sets the next "hole" warp destination (`sFixedHoleWarp`), but doesn't actually send the player anywhere. This destination may potentially be used by a later call to `warphole`.


### getplayerxy
**Opcode:** 0x42

### getpartysize
**Opcode:** 0x43

### additem
**Opcode:** 0x44  
**Syntax:** `additem <variable-or-value item-type> <variable-or-value quantity>`

Attempts to add an item to the player's bag, making changes only if the bag has room for <var>quantity</var> many of that item. `VAR_RESULT` (variable 0x800C) is set to 1 if the operation succeeds, or 0 otherwise.


### removeitem
**Opcode:** 0x45  
**Syntax:** `removeitem <variable-or-value item-type> <variable-or-value quantity>`

Attempts to remove an item to the player's bag, making changes only if the bag contains at least <var>quantity</var> many of that item. `VAR_RESULT` (variable 0x800C) is set to 1 if the operation succeeds, or 0 otherwise.


### checkitemspace
**Opcode:** 0x46  
**Syntax:** `checkitemspace <variable-or-value item-type> <variable-or-value quantity>`

Checks whether the player's bag has room for <var>quantity</var> many more of the specified item type, setting `VAR_RESULT` (variable 0x800C) to 1 if so or 0 if not.


### checkitem
**Opcode:** 0x47  
**Syntax:** `checkitem <variable-or-value item-type> <variable-or-value quantity>`

Checks whether the player's bag contains at least <var>quantity</var> many of the specified item type, setting `VAR_RESULT` (variable 0x800C) to 1 if so or 0 if not.


### checkitemtype
**Opcode:** 0x48  
**Syntax:** `checkitemtype <variable-or-value item-type>`

Sets `VAR_RESULT` (variable 0x800C) to the bag pocket ID for the given item type (that is, `gItems[item_type].pocket`).


### addpcitem
**Opcode:** 0x49  
**Syntax:** `addpcitem <variable-or-value item-type> <variable-or-value quantity>`

Attempts to add an item to the player's PC, making changes only if the PC has room for <var>quantity</var> many of that item. `VAR_RESULT` (variable 0x800C) is set to 1 if the operation succeeds, or 0 otherwise.


### checkpcitem
**Opcode:** 0x4A  
**Syntax:** `checkpcitem <variable-or-value item-type> <variable-or-value quantity>`

Checks whether the player's PC contains at least <var>quantity</var> many of the specified item type, setting `VAR_RESULT` (variable 0x800C) to 1 if so or 0 if not.


### adddecoration
**Opcode:** 0x4B

### removedecoration
**Opcode:** 0x4C

### checkdecor
**Opcode:** 0x4D

### checkdecorspace
**Opcode:** 0x4E

### applymovement
**Opcode:** 0x4F

### applymovementat
**Opcode:** 0x50

### waitmovement
**Opcode:** 0x51

### waitmovementat
**Opcode:** 0x52

### removeobject
**Opcode:** 0x53

### removeobjectat
**Opcode:** 0x54

### addobject
**Opcode:** 0x55

### addobjectat
**Opcode:** 0x56

### setobjectxy
**Opcode:** 0x57

### showobjectat
**Opcode:** 0x58

### hideobjectat
**Opcode:** 0x59

### faceplayer
**Opcode:** 0x5A

### turnobject
**Opcode:** 0x5B

### trainerbattle
**Opcode:** 0x5C

### dotrainerbattle
**Opcode:** 0x5D

### gotopostbattlescript
**Opcode:** 0x5E

### gotobeatenscript
**Opcode:** 0x5F

### checktrainerflag
**Opcode:** 0x60

### settrainerflag
**Opcode:** 0x61

### cleartrainerflag
**Opcode:** 0x62

### setobjectxyperm
**Opcode:** 0x63

### copyobjectxytoperm
**Opcode:** 0x64

### setobjectmovementtype
**Opcode:** 0x65

### waitmessage
**Opcode:** 0x66

### message
**Opcode:** 0x67

### closemessage
**Opcode:** 0x68

### lockall
**Opcode:** 0x69

### lock
**Opcode:** 0x6A

### releaseall
**Opcode:** 0x6B

### release
**Opcode:** 0x6C

### waitbuttonpress
**Opcode:** 0x6D

### yesnobox
**Opcode:** 0x6E

### multichoice
**Opcode:** 0x6F

### multichoicedefault
**Opcode:** 0x70

### multichoicegrid
**Opcode:** 0x71

### drawbox
**Opcode:** 0x72

### erasebox
**Opcode:** 0x73

### drawboxtext
**Opcode:** 0x74

### showmonpic
**Opcode:** 0x75

### hidemonpic
**Opcode:** 0x76

### showcontestpainting
**Opcode:** 0x77

### braillemessage
**Opcode:** 0x78

### givemon
**Opcode:** 0x79

### giveegg
**Opcode:** 0x7A

### setmonmove
**Opcode:** 0x7B

### checkpartymove
**Opcode:** 0x7C

### bufferspeciesname
**Opcode:** 0x7D

### bufferleadmonspeciesname
**Opcode:** 0x7E

### bufferpartymonnick
**Opcode:** 0x7F

### bufferitemname
**Opcode:** 0x80

### bufferdecorationname
**Opcode:** 0x81

### buffermovename
**Opcode:** 0x82

### buffernumberstring
**Opcode:** 0x83

### bufferstdstring
**Opcode:** 0x84

### bufferstring
**Opcode:** 0x85

### pokemart
**Opcode:** 0x86

### pokemartdecoration
**Opcode:** 0x87

### pokemartdecoration2
**Opcode:** 0x88

### playslotmachine
**Opcode:** 0x89

### setberrytree
**Opcode:** 0x8A

### choosecontestmon
**Opcode:** 0x8B

### startcontest
**Opcode:** 0x8C

### showcontestresults
**Opcode:** 0x8D

### contestlinktransfer
**Opcode:** 0x8E

### random
**Opcode:** 0x8F  
**Syntax:** `random <variable-or-value max>`

Sets `VAR_RESULT` (variable 0x800C) to a random number in the range [0, <var>max</var>).


### addmoney
**Opcode:** 0x90

### removemoney
**Opcode:** 0x91

### checkmoney
**Opcode:** 0x92

### showmoneybox
**Opcode:** 0x93

### hidemoneybox
**Opcode:** 0x94

### updatemoneybox
**Opcode:** 0x95

### getpokenewsactive
**Opcode:** 0x96

### fadescreen
**Opcode:** 0x97  
**Syntax:** `fadescreen <u8 mode>`

Fades the screen out, halting script execution until the fade completes.

Valid fade modes are:  
<dl>
   <dt>0</dt>
      <dd><code>FADE_FROM_BLACK</code> in <code>include/constants/field_weather.h</code>.</dd>
   <dt>1</dt>
      <dd><code>FADE_TO_BLACK</code> in <code>include/constants/field_weather.h</code>.</dd>
   <dt>2</dt>
      <dd><code>FADE_FROM_WHITE</code> in <code>include/constants/field_weather.h</code>.</dd>
   <dt>3</dt>
      <dd><code>FADE_TO_WHITE</code> in <code>include/constants/field_weather.h</code>.</dd>
</dl>


### fadescreenspeed
**Opcode:** 0x98  
**Syntax:** `fadescreen <u8 mode> <u8 frame-delay>`

The same as `fadescreenspeed`, except that the fade is slowed down by delaying for <var>frame-delay</var> extra frames at each step of the fade.


### setflashlevel
**Opcode:** 0x99

### animateflash
**Opcode:** 0x9A

### messageautoscroll
**Opcode:** 0x9B

### dofieldeffect
**Opcode:** 0x9C

### setfieldeffectargument
**Opcode:** 0x9D

### waitfieldeffect
**Opcode:** 0x9E

### setrespawn
**Opcode:** 0x9F

### checkplayergender
**Opcode:** 0xA0

### playmoncry
**Opcode:** 0xA1

### setmetatile
**Opcode:** 0xA2

### resetweather
**Opcode:** 0xA3  
**Syntax:** `resetweather`

Resets the current weather to that specified by the current map's header. Internally, this calls `SetSavedWeatherFromCurrMapHeader()`.


### setweather
**Opcode:** 0xA4  
**Syntax:** `setweather <variable-or-value>`

Sets the current weather, but does not actually apply it (see `doweather`). Valid weather values have their names listed in `include/constants/weather.h` (the `WEATHER_` enum).


### doweather
**Opcode:** 0xA5  
**Syntax:** `doweather`

Applies the current set weather via a call to `DoCurrentWeather()`.


### setstepcallback
**Opcode:** 0xA6  
**Syntax:** `setstepcallback <u8 index>`

Selects a hardcoded function from a predefined list (`sPerStepCallbacks` in `src/field_tasks.c`), and sets that function to run every time the player takes a step. If the index you use is out of bounds, then a dummy function that does nothing is used.

Under the hood, this calls `ActivatePerStepCallback(index)`.

In the vanilla game, the available step functions are:

| Index | Name | Description |
| -: | - | :- |
| 0 |  `STEP_CB_DUMMY` |  Does nothing.
| 1 | `STEP_CB_ASH` |  Handles knocking fallen ash loose from tall grass as you walk through it.
| 2 | `STEP_CB_FORTREE_BRIDGE` |  Handles Fortree bridges reacting to the player's weight.
| 3 | `STEP_CB_PACIFIDLOG_BRIDGE` |  Handles Fortree bridges reacting to the player's weight.
| 4 | `STEP_CB_SOOTOPOLIS_ICE` |  Handles the cracking ice floors in Sootopolis City's Gym.
| 5 | `STEP_CB_TRUCK` | 
| 6 | `STEP_CB_SECRET_BASE` | 
| 7 | `STEP_CB_CRACKED_FLOOR` | Handles the cracking floors in places like Sky Pillar.


### setmaplayoutindex
**Opcode:** 0xA7  
**Syntax:** `setmaplayout <variable-or-value>`

Sets the current map layout (size, tileset, tile data, et cetera) to that of another map, overriding the map's natural layout. This seems to be used for the Battle Pyramid.


### setobjectsubpriority
**Opcode:** 0xA8

### resetobjectsubpriority
**Opcode:** 0xA9

### createvobject
**Opcode:** 0xAA

### turnvobject
**Opcode:** 0xAB

### opendoor
**Opcode:** 0xAC

### closedoor
**Opcode:** 0xAD

### waitdooranim
**Opcode:** 0xAE

### setdooropen
**Opcode:** 0xAF

### setdoorclosed
**Opcode:** 0xB0

### addelevmenuitem
**Opcode:** 0xB1

### showelevmenu
**Opcode:** 0xB2

### checkcoins
**Opcode:** 0xB3

### addcoins
**Opcode:** 0xB4

### removecoins
**Opcode:** 0xB5

### setwildbattle
**Opcode:** 0xB6

### dowildbattle
**Opcode:** 0xB7

### setvaddress
**Opcode:** 0xB8

### vgoto
**Opcode:** 0xB9

### vcall
**Opcode:** 0xBA

### vgoto_if
**Opcode:** 0xBB

### vcall_if
**Opcode:** 0xBC

### vmessage
**Opcode:** 0xBD

### vbuffermessage
**Opcode:** 0xBE

### vbufferstring
**Opcode:** 0xBF

### showcoinsbox
**Opcode:** 0xC0

### hidecoinsbox
**Opcode:** 0xC1

### updatecoinsbox
**Opcode:** 0xC2

### incrementgamestat
**Opcode:** 0xC3

### setescapewarp
**Opcode:** 0xC4  
**Syntax:** `setdynamicwarp <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Sets the next Escape Rope warp destination (`gSaveBlock1Ptr->escapeWarp`), but doesn't actually send the player anywhere.


### waitmoncry
**Opcode:** 0xC5

### bufferboxname
**Opcode:** 0xC6

### setmonmodernfatefulencounter
**Opcode:** 0xCD

### checkmonmodernfatefulencounter
**Opcode:** 0xCE

### trywondercardscript
**Opcode:** 0xCF

### warpspinenter
**Opcode:** 0xD1

### setmonmetlocation
**Opcode:** 0xD2

### moverotatingtileobjects
**Opcode:** 0xD3

### turnrotatingtileobjects
**Opcode:** 0xD4

### initrotatingtilepuzzle
**Opcode:** 0xD5

### freerotatingtilepuzzle
**Opcode:** 0xD6

### warpmossdeepgym
**Opcode:** 0xD7  
**Syntax:** `warpteleport <s8 map-group> <s8 map-num> <s8 warp-id> <variable-or-value x> <variable-or-value y>`

Similar to `warp`, but under the hood, `DoMossdeepGymWarp` is used instead of `DoWarp`.


### selectapproachingtrainer
**Opcode:** 0xD8

### lockfortrainer
**Opcode:** 0xD9

### closebraillemessage
**Opcode:** 0xDA

### messageinstant
**Opcode:** 0xDB

### fadescreenswapbuffers
**Opcode:** 0xDC  
**Syntax:** `fadescreenswapbuffers <u8 mode>`

The same as `fadescreenspeed`, except that fades to black or white begin by copying palette data from <code>gPlttBufferUnfaded</code> to <code>gPaletteDecompressionBuffer</code>, while fades from black or white begin by copying data in the opposite direction.


### buffertrainerclassname
**Opcode:** 0xDD

### buffertrainername
**Opcode:** 0xDE

### pokenavcall
**Opcode:** 0xDF

### warpwhitefade
**Opcode:** 0xE0

### buffercontestname
**Opcode:** 0xE1

### bufferitemnameplural
**Opcode:** 0xE2

