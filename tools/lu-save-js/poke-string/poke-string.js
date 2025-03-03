class PokeString {
   constructor() {
      this.bytes = [];
   }
   
   get length() {
      return this.bytes.length;
   }
   
   // May throw.
   static /*void*/ from_text(/*String*/ str) {
      const out = new PokeString();
      for(let i = 0; i < str.length; ++i) {
         let c = str[i];
         if (c == '\\') {
            if (str.length == i + 1)
               throw new Error("Truncated escape sequence.");
            let d = str[i + 1];
            switch (d) {
               case '\\':
                  out.bytes.push(CHARSET_CONTROL_CODES.chars_to_bytes['\\']);
                  ++i;
                  continue;
               case "0":
                  out.bytes.push(CHARSET_CONTROL_CODES.chars_to_bytes["\0"]);
                  ++i;
                  continue;
               case "l":
               case "p":
               case "n":
                  out.bytes.push(CHARSET_CONTROL_CODES.chars_to_bytes[c + d]);
                  ++i;
                  continue;
            }
            if (d != "x") {
               throw new Error(`Invalid escape sequence (\${d}...) at position ${i}.`);
            }
            if (i + 3 >= str.length)
               throw new Error("Truncated escape sequence.");
            let ch = parseInt(str[i + 2] + str[i + 3], 16);
            if (isNaN(ch))
               throw new Error(`Invalid character-code escape sequence (${str.substring(i, i + 4)}) at position ${i}.`);
            out.bytes.push(ch);
            i += 3;
            continue;
         }
         if (c == '&') {
            let j = str.indexOf(';', i + 1);
            if (j < 0)
               throw new Error(`Truncated XML entity at position ${i}.`);
            let entity = str.substring(i + 1, j);
            let bytes  = CHARSET_ENGLISH.get_entity(entity);
            if (!bytes) {
               bytes = CHARSET_CONTROL_CODES.get_entity(entity);
               if (!bytes)
                  throw new Error(`Unrecognized XML entity ${str.substring(i, j + 1)} at position ${i}.`);
            }
            out.bytes = out.bytes.concat(bytes);
            i = j;
            continue;
         }
         let cc = CHARSET_ENGLISH.chars_to_bytes[c];
         if (cc === null) {
            cc = CHARSET_CONTROL_CODES.chars_to_bytes[c];
            if (cc === null)
               throw new Error("Character code 0x${c.charCodeAt(0).toString(16).toUpperCase()} is not representable in the game's encodings.");
         }
         out.bytes.push(cc);
      }
      return out;
   }
};