
# Data translation

One of the features of this save editor is the ability to translate a savegame from one bitpacked data format to another. The goal is to allow players to seamlessly update the modded game without having to abandon their current playthrough.

## Creating a user-defined translator

User-defined translators are subclasses of `AbstractDataFormatTranslator`. They are expected to override the `translateInstance` method. This override receives two arguments: a source `CInstance` and a destination `CInstance`. The translator is expected to review the data in the source, and copy that data (altering it in whatever manner is necessary) into the destination. (Modifying the source is undefined behavior.)

Translators are only invoked when the destination value isn't fully filled in. There's no guarantee that the source and destination objects will be of the same class; for example, even if `abc.xyz[3][2]` is a `CValueInstance` in the source format, it may be a `CStructInstance` or something else in the destination format.

The translator *must* handle all data (besides `omitted` fields) in the destination `CInstance` and its nested `CInstance`s, by either writing values in, or by passing a `CInstance` to `this.pass`. Calling `this.pass` is how you decline to handle a given value: the translation system will see that and carry out any default behaviors (e.g. calling other applicable translators, or attempting default value copying/conversion). We require that you either fill in or pass a value, as a safeguard to help make it harder to forget to fill something in.

* Translation fails if you leave any data in the destination empty (i.e. neither setting a value nor calling `this.pass`).

* Calling `this.pass` on a `CInstance` representing an aggregate (i.e. `CStructInstance`, `CUnionInstance`, or `CValueInstanceArray`) will recursively pass all members of the aggregate.

* Don't overwrite the `CInstance` itself. For a `CValueInstance`, overwrite its `value`; for other `CInstance` types (representing aggregates), deal with their members as appropriate.

* You can still write values into a `CInstance` after you `pass` on handling it.

### An example translator

This code uses a translator to run the ROT13 cipher on the player's name.

```js
class layerNameROT13Translator extends AbstractDataFormatTranslator {
   translateInstance(src, dst) {
      //
      // We want to translate a specific field, so we `pass` the entire 
      // `dst` object and then write in just the values we want. This 
      // will allow the default handling to run for everything we don't 
      // fill in.
      //
      this.pass(dst);
      //
      // We've passed on handling `dst`, but we still *can* handle it or 
      // any of its sub-objects. In this case, the source and destination 
      // objects are CStructInstances, so we can access their `members`:
      //
      let src_name = src.members["playerName"].value;
      let dst_inst = dst.members["playerName"];
      let dst_name = dst_inst.value = new PokeString();
      for(let i = 0; i < src_name.length; ++i) {
         let byte = src_name.bytes[i];
         if (byte >= 0xBB && byte <= 0xEE) {
            let base   = 0;
            let letter = byte - 0xBB;
            if (letter >= 26) {
               letter -= 26;
               base   += 26;
            }
            letter = (letter + 13) % 26;
            byte = 0xBB + base + letter;
         }
         dst_name.bytes.push(byte);
      }
   }
};

//
// Instantiate the translation, and then attach it to a TranslationOperation 
// that we're about to run. We're asking to handle the SaveBlock2 data.
//
let tran = new PlayerNameROT13Translator();
operation.translators_for_top_level_values.add("dst", "gSaveBlock2Ptr", tran);
```

## Savedata handled by a translator

The following `CInstance` subclasses exist:

### `CValueInstance`

This object represents a non-aggregate variable or field within the savedata: a boolean, an integer, a buffer, a string, et cetera. Simply put: everything is either a `CValueInstance` or some kind of group ("aggregate") of `CValueInstance`s.

The `value` property is where the data is stored.

You can check the variable/field definition by accessing `inst.decl`. This includes checking the serialized category of the value (`inst.decl.type`): a string with one of the following values: `boolean`, `buffer`, `integer`, `omitted`, `pointer`, or `string`. You generally shouldn't need to do this, because you should be creating translators to handle specific data conversions; but the option is there.

If your translator chooses to fill in a value, that value must be of the appropriate type:

* No requirements are placed on `omitted` fields.
* A `boolean` field must be given a boolean or an integer (which will be implicitly converted).
* A `buffer` field must be given an instance of `DataView`. The view's length cannot exceed the maximum bytecount for the opaque buffer field.
* An `integer` field must be given an integer. The integer must lie within the bounds of the integer field.
* A `pointer` field must be given a value that can fit in a 32-bit integer. We allow signed values because bitwise operations in JavaScript coerce the result to a signed value.
* A `string` field must be given an instance of `PokeString`. The string must fit within the maximum length of the string field.

### `CValueInstanceArray`

This object represents an aggregate -- specifically, an array. The elements of this array are represented via `inst.values`, an `Array` of `CInstance` objects which may be single `CValueInstance`s or nested aggregates.

Your translator should not modify `inst.values` directly: don't `push` or `pop` values, don't replace the array, don't replace its elements, and don't alter the array's length. Just modify the elements (`CInstance`s) that are already in the array.

### `CStructInstance`

This object represents an aggregate -- specifically, a struct. You can access the `CInstance` for any struct member via `inst.members[name]`.

### `CUnionInstance`

This object represents an aggregate -- specifically, a tagged union. The active union member is a `CInstance` accessible via `inst.value`.

If an externally-tagged union is passed to your translator as a destination `CInstance`, the union will have its active member already created unless the union's tag was set to a value that didn't match any of the union's members. If the union is internally-tagged, then its active member will always be missing (since the tag would be inside of it).

## Other relevant classes

### `PokeString`

A class for strings within savedata. Since Game Freak uses custom text encodings, this is defined as a class that wraps an array of bytes; string literals are not acceptable values here.