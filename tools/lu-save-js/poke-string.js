class PokeString {
   constructor() {
      this.bytes = [];
   }
   
   get length() {
      return this.bytes.length;
   }
   
   /*String*/ to_text() /*const*/ {
      let out = "";
      for(let cc of this.bytes) {
         if (cc === CHARSET_CONTROL_CODES.chars_to_bytes["\0"])
            break;
         let ch = CHARSET_ENGLISH.bytes_to_chars[cc];
         if (ch && ch.length == 1) {
            if (ch == '\\')
               out += '\\';
            out += ch;
            continue;
         }
         out += "\\x" + cc.toString(16).toUpperCase().padStart(2, '0');
      }
      return out;
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
   
   
   /*DocumentFragment*/ to_dom() /*const*/ {
      let frag = new DocumentFragment();
      let text = "";
      
      function _append_node(node) {
         if (text) {
            frag.append(text);
            text = "";
         }
         frag.append(node);
      }
      
      for(let i = 0; i < this.bytes.length; ++i) {
         let cc = this.bytes[i];
         if (cc === CHARSET_CONTROL_CODES.chars_to_bytes["\0"])
            break;
         let ch = CHARSET_ENGLISH.bytes_to_chars[cc];
         if (ch && ch.length == 1) {
            text += ch;
            continue;
         }
         switch (cc) {
            case 0xF8: // keypad icons
               {
                  let icon_name = [
                     "a",
                     "b",
                     "l",
                     "r",
                     "start",
                     "select",
                     "dpad-up",
                     "dpad-down",
                     "dpad-left",
                     "dpad-right",
                     "dpad-up-down",
                     "dpad-left-right",
                     "dpad",
                  ][this.bytes[i + 1]];
                  if (icon_name) {
                     icon_name = "keypad-" + icon_name;
                     
                     let node = document.createElement("div");
                     node.classList.add("icon");
                     node.setAttribute("data-icon", icon_name);
                     frag.append(node);
                  }
                  ++i;
               }
               break;
            case 0xF9: // extra symbols
               {
                  let arg = this.bytes[i + 1];
                  if (arg >= 0x00 && arg <= 0x04) {
                     text += [
                        "\u1F845", // thick arrow up
                        "\u1F847", // ...down
                        "\u1F844", // ...left
                        "\u1F846", // ...right
                        "\uFF0B",  // fullwidth plus
                     ][arg];
                     ++i;
                     continue;
                  }
                  if (arg >= 0x05 && arg <= 0x08) {
                     let node = document.createElement("span");
                     node.classList.add("ligature");
                     node.textContent = [
                        "Lv",
                        "PP",
                        "ID",
                        "\u2116", // "No."
                     ][arg - 0x05];
                     _append_node(node);
                     ++i;
                     continue;
                  }
                  if (arg == 0x09) {
                     text += "_";
                     ++i;
                     continue;
                  }
                  if (arg >= 0x0A && arg <= 0x12) {
                     text += String.fromCharCode(0x2780 + arg - 0x0A);
                     ++i;
                     continue;
                  }
                  if (arg >= 0x13 && arg <= 0x17) {
                     text += [
                        "(",
                        ")",
                        "\u25CE", // bullseye (may be \u2B57 chizukigou for a city hall?)
                        "\u25B3", // triangle outline
                        "\u00D7", // multiply
                     ][arg - 0x13];
                     ++i;
                     continue;
                  }
                  
                  if (arg < 0xD0) {
                     ++i;
                     continue;
                  }
                  arg -= 0xD0;
                  {
                     let ch = [
                        "\uFF3F", // fullwidth underscore
                        "\uFF5C", // fullwidth vertical bar
                        "\uFFE3", // fullwidth macron
                        "\u2053", // swung dash (similar to a tilde)
                        "\uFF08", // fullwidth left paren
                        "\uFF09", // fullwidth right paren
                        "\u2282", // union
                        ">",      // greater than
                        null, // eye, left
                        null, // eye, right
                        "@",
                        "\uFF1B", // fullwidth semicolon
                        "\u2795", // heavy plus
                        "\u2796", // heavy minus
                        "\uFF1D", // fullwidth equals (or \u3013 geta kigō?)
                        null, // spiral (icon)
                        "👅", // tongue
                        "\u25B3", // triangle outline
                        "´",
                        "`",
                        "\u25CF", // black circle
                        "\u25BC", // black triangle
                        "\u25A0", // black square
                        "\u2764\uFE0E", // black heart
                        "\u1F319", // crescent moon
                        "\u266A", // music note
                        "\u25D3", // circle, upper half shaded
                        "\u26A1\uFE0E", // lightning bolt (note: in-game is mirrored)
                        "\u1F342", // leaf, pointing upward
                        "\u1F525", // fire
                        "\u1F4A7", // water
                        "\u1F91B", // fist, left-facing
                        "\u1F91C", // fist, right-facing
                        "\u2741",  // "big wheel" in decomp; "flower" in Bulbapedia
                        "\uA669",  // "small wheel" in decomp; "eye" in Bulbapedia
                        "\u1F441", // "sphere" in decomp; "eye" in Bulbapedia
                        "\u1F4A2", // anger mark
                        null,      // emoji: mischievous
                        null,      // emoji: happy
                        null,      // emoji: angry
                        null,      // emoji: surprised
                        null,      // emoji: big smile
                        null,      // emoji: evil
                        null,      // emoji: tired
                        null,      // emoji: neutral
                        null,      // emoji: shocked
                        null,      // emoji: rage
                     ][arg];
                     if (ch) {
                        text += ch;
                        ++i;
                        continue;
                     }
                     arg -= 2;
                     
                  }
                  let icon_name;
                  if (!icon_name) {
                     let v = arg - 0xF5;
                     if (v >= 0) {
                        icon_name = [
                           "emoji-mischievous",
                           "emoji-happy",
                           "emoji-angry",
                           "emoji-surprised",
                           "emoji-big-smile",
                           "emoji-evil",
                           "emoji-tired",
                           "emoji-neutral",
                           "emoji-shocked",
                           "emoji-rage",
                        ][v];
                     }
                  }
                  if (icon_name) {
                     icon_name = icon_name;
                     
                     let node = document.createElement("div");
                     node.classList.add("icon");
                     node.setAttribute("data-icon", icon_name);
                     frag.append(node);
                  }
                  ++i;
               }
               break;
            case 0xFC: // control codes
               if (i == this.bytes.length - 1) {
                  continue;
               } else {
                  let byte = this.bytes[i + 1];
                  switch (byte) {
                     //
                     // Within this switch-case, add to `i` the total size in 
                     // bytes of the control code and its arguments, including 
                     // the leading 0xFC byte.
                     //
                     case 0x00: // location end marker
                        i += 2;
                        break;
                     case 0x01: // set color
                     case 0x02: // set highlight
                     case 0x03: // set shadow
                        i += 3;
                        break;
                     case 0x04: // set color, highlight, and shadow
                        i += 5;
                        break;
                     case 0x05: // set palette
                     case 0x06: // set font
                        i += 3;
                        break;
                     case 0x07: // reset font
                        i += 2;
                        break;
                     case 0x08: // pause (byte argument)
                        i += 3;
                        break;
                     case 0x09: // pause until press
                     case 0x0A: // wait for sound effect
                        i += 2;
                        break;
                     case 0x0B: // play music
                        i += 4;
                        break;
                     case 0x0C: // escape
                        {
                           let arg = this.bytes[i + 2];
                           //
                           // Functionally the same as `0xF9 <arg>`.
                           //
                           // TODO: ???
                           //
                        }
                        i += 3; // ?
                        break;
                     case 0x0D: // shift right
                     case 0x0E: // shift down
                        //
                        // TODO
                        //
                        i += 3;
                        break;
                     case 0x0F: // fill window
                        frag = new DocumentFragment();
                        i += 2;
                        break;
                     case 0x10: // play sound effect
                        i += 4;
                        break;
                     case 0x11: // clear X many pixels ahead
                        //
                        // TODO: Only useful in combination with "shift right," to "rewind" 
                        // the text printing and print something else?
                        //
                        {
                           let arg = this.bytes[i + 2];
                           let spacer = document.createElement("span");
                           spacer.classList.add("spacer");
                           spacer.style.width = `${arg}px`;
                           frag.append(spacer);
                        }
                        i += 3;
                        break;
                     case 0x12: // skip X many pixels ahead
                        {
                           let arg = this.bytes[i + 2];
                           let spacer = document.createElement("span");
                           spacer.classList.add("spacer");
                           spacer.style.width = `${arg}px`;
                           frag.append(spacer);
                        }
                        i += 3;
                        break;
                     case 0x13: // clear to X-coordinate
                        {
                           let arg = this.bytes[i + 2];
                           //
                           // TODO: How would we even implement this...?
                           //
                        }
                        i += 3;
                        break;
                     case 0x14: // minimum letter spacing
                        {
                           let arg = this.bytes[i + 2];
                           //
                           // TODO: How would we even implement this...?
                           //
                        }
                        i += 3;
                        break;
                     case 0x15: // to Japanese
                        //
                        // TODO
                        //
                        i += 2;
                        break;
                     case 0x16: // to Latin
                        //
                        // TODO
                        //
                        i += 2;
                        break;
                     case 0x17: // pause music
                     case 0x18: // resume music
                        i += 2;
                        break;
                  }
                  --i;
                  continue;
               }
               break;
            case 0xFD: // string substitution
               if (i == this.bytes.length - 1) {
                  continue;
               } else {
                  let byte = this.bytes[i + 1];
                  switch (byte) {
                     case 0x01:
                     case 0x02:
                     case 0x03:
                     case 0x04:
                     case 0x05:
                     case 0x06:
                        {
                           let node = document.createElement("span");
                           node.classList.add("runtime-substitution");
                           node.textContent = [
                              "PLAYER",
                              "STR_VAR_1",
                              "STR_VAR_2",
                              "STR_VAR_3",
                              "KUN",
                              "RIVAL",
                           ][byte - 1];
                           frag.append(node);
                        }
                        break;
                     case 0x07:
                        frag.append("EMERALD");
                        break;
                     case 0x08:
                        frag.append("AQUA");
                        break;
                     case 0x09:
                        frag.append("MAGMA");
                        break;
                     case 0x0A:
                        frag.append("ARCHIE");
                        break;
                     case 0x0B:
                        frag.append("MAXIE");
                        break;
                     case 0x0C:
                        frag.append("KYOGRE");
                        break;
                     case 0x0D:
                        frag.append("GROUDON");
                        break;
                  }
               }
               ++i;
               break;
         }
         
      }
      if (text)
         frag.append(text);
      return frag;
   }
};