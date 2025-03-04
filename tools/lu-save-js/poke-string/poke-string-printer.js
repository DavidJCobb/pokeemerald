//
// A class that does the work of processing string formatting and substitution 
// codes for you. Subclass it and override the various handlers in order to 
// format a PokeString however you please.
//
class AbstractPokeStringPrinter {
   constructor() {
   }
   
   print(/*const PokeString*/ str) {
      this.pre_print_handler();
      let   i    = 0;
      const size = str.length;
      if (size == 0) {
         this.post_print_handler();
         return;
      }
      do {
         let c = str.bytes[i];
         switch (c) {
            case 0xF8:
               this.handle_keypad_icon(str.bytes[i + 1]);
               i += 2;
               continue;
            case 0xF9:
               this.handle_extra_symbol(str.bytes[i + 1], c);
               i += 2;
               continue;
            case 0xFC:
               {
                  let cmd = str.bytes[i + 1];
                  switch (cmd) {
                     case 0x00:
                        this.handle_ctrl_location_end();
                        break;
                     case 0x01:
                     case 0x02:
                     case 0x03:
                     case 0x05:
                        this.handle_ctrl_set_color(
                           ["color", "highlight", "shadow", null, "palette"][cmd],
                           str.bytes[i + 2]
                        );
                        i += 3;
                        continue;
                     case 0x04:
                        {
                           let color     = str.bytes[i + 2];
                           let highlight = str.bytes[i + 3];
                           let shadow    = str.bytes[i + 4];
                           this.handle_ctrl_set_all_colors(color, highlight, shadow);
                           i += 5;
                        }
                        continue;
                     case 0x06:
                        this.handle_ctrl_set_font(str.bytes[i + 2]);
                        i += 3;
                        continue;
                     case 0x07:
                        this.handle_ctrl_reset_font();
                        break;
                     case 0x08:
                        this.handle_ctrl_pause(str.bytes[i + 2]);
                        i += 3;
                        continue;
                     case 0x09:
                        this.handle_ctrl_pause_until_press();
                        break;
                     case 0x0A:
                        this.handle_ctrl_wait_for_sound();
                        break;
                     case 0x0B:
                        {
                           let index = (str.bytes[i + 2] << 8) | str.bytes[i + 3];
                           this.handle_ctrl_play_music(index);
                        }
                        i += 4;
                        continue;
                     case 0x0C:
                        this.handle_extra_symbol(str.bytes[i + 2], c);
                        i += 3;
                        continue;
                     case 0x0D:
                     case 0x0E:
                        this.handle_ctrl_shift(cmd == 0x0D ? "x" : "y", str.bytes[i + 2]);
                        i += 3;
                        continue;
                     case 0x0F:
                        this.handle_ctrl_fill_window();
                        break;
                     case 0x10:
                        {
                           let index = (str.bytes[i + 2] << 8) | str.bytes[i + 3];
                           this.handle_ctrl_play_sound(index);
                        }
                        i += 4;
                        continue;
                     case 0x11:
                        this.handle_ctrl_clear_pixels_ahead(str.bytes[i + 2]);
                        i += 3;
                        continue;
                     case 0x12:
                        this.handle_ctrl_skip_pixels(str.bytes[i + 2]);
                        i += 3;
                        continue;
                     case 0x13:
                        this.handle_ctrl_clear_to_x_coordinate(str.bytes[i + 2]);
                        i += 3;
                        continue;
                     case 0x14:
                        this.handle_ctrl_set_letter_spacing(str.bytes[i + 2]);
                        i += 3;
                        continue;
                     case 0x15:
                     case 0x16:
                        this.handle_ctrl_set_character_set(cmd == 0x15 ? "japanese" : "latin");
                        break;
                     case 0x17:
                        this.handle_ctrl_pause_music();
                        break;
                     case 0x18:
                        this.handle_ctrl_resume_music();
                        break;
                     default:
                        this.handle_raw_byte(c);
                        this.handle_raw_byte(cmd);
                        break;
                  }
                  i += 2;
                  continue;
               }
               break;
            case 0xFD:
               this.handle_string_substitution(str.bytes[i + 1]);
               i += 2;
               continue;
            case 0xFF:
               this.post_print_handler();
               return;
         }
         this.handle_char(c);
         ++i;
      } while (i < size);
      this.post_print_handler();
   }
   
   pre_print_handler() {}
   
   // for invalid control codes, etc.
   handle_raw_byte(value) {}
   
   handle_char(char_code) {}
   
   // F8 xx
   handle_keypad_icon(index) {}
   
   // F9 xx or FC 0C xx
   // `escape_type` is 0xF9 or 0xFC
   handle_extra_symbol(index, escape_type) {
      //
      // For simply displaying a string, it's generally fine to just defer 
      // to this implementation here. If you want to e.g. display escape 
      // sequences for the raw bytes, then you'd override this.
      //
      this.handle_char(0x0100 | index);
   }
   
   // FC 00
   handle_ctrl_location_end() {}
   
   // FC xx yy
   //  - xx == 01: color
   //  - xx == 02: highlight
   //  - xx == 03: shadow
   //  - xx == 05: palette
   handle_ctrl_set_color(type, index) {}
   
   // FC 04 xx yy zz
   handle_ctrl_set_all_colors(color, highlight, shadow) {}
   
   // FC 06 xx
   handle_ctrl_set_font(index) {}
   
   // FC 07
   handle_ctrl_reset_font() {}
   
   // FC 08 xx
   handle_ctrl_pause(delay) {}
   
   // FC 09
   handle_ctrl_pause_until_press() {}
   
   // FC 0A
   handle_ctrl_wait_for_sound() {}
   
   // FC 0B xx xx
   handle_ctrl_play_music(index) {}
   
   // FC 0D xx
   // FC 0E yy
   handle_ctrl_shift(axis, by) {}
   
   // FC 0F
   handle_ctrl_fill_window() {}
   
   // FC 10 xx xx
   handle_ctrl_play_sound(index) {}
   
   // FC 11 xx
   // Only useful in combination with "shift right," to "rewind" the text and 
   // print something else?
   handle_ctrl_clear_pixels_ahead(by) {}
   
   // FC 12 xx
   handle_ctrl_skip_pixels(x) {}
   
   // FC 13 xx
   // Clear pixels up to some X-coordinate, on the current line. The coordinate 
   // is relative to the X-position of the region we're printing to.
   handle_ctrl_clear_to_x_coordinate(x) {}
   
   // FC 14 xx
   handle_ctrl_set_letter_spacing(v) {}
   
   // FC 15 for "japanese"
   // FC 16 for "latin"
   handle_ctrl_set_character_set(what) {}
   
   // FC 17
   handle_ctrl_pause_music() {}
   
   // FC 18
   handle_ctrl_resume_music() {}
   
   // FD xx
   handle_string_substitution(index) {}
   
   post_print_handler() {}
};

class LiteralPokeStringPrinter extends AbstractPokeStringPrinter {
   #current_charset;
   constructor() {
      super();
      this.charset       = "latin";
      this.escape_quotes = false;
      //
      this.result = "";
   }
   pre_print_handler() {
      this.#current_charset = this.charset;
   }
   post_print_handler() {
      this.#current_charset = this.charset;
   }
   
   #format_byte(b) {
      return (+b).toString(16).toUpperCase().padStart(2, '0');
   }
   #format_word(w) {
      let hi = this.#format_byte(w >> 16);
      let lo = this.#format_byte(w & 0xFF);
      return `\\x${hi}\\x${lo}`;
   }
   
   handle_raw_byte(value) {
      this.result += `\\x${this.#format_byte(value)}`;
   }
   
   handle_char(cc) {
      let ch = CHARMAP.codepoint_to_character(cc, this.#current_charset);
      if (ch && ch.length == 1) {
         switch (ch) {
            case '\\':
               this.result += "\\\\";
               return;
            case '\v':
               this.result += "\\l";
               return;
            case '\f':
               this.result += "\\p";
               return;
            case '\n':
               this.result += "\\n";
               return;
            case '"':
               if (this.escape_quotes) {
                  this.result += "\\\"";
                  return;
               }
               break;
         }
         this.result += ch;
         return;
      }
      this.result += `\\x${this.#format_byte(cc)}`;
   }
   
   handle_keypad_icon(index) {
      this.result += `\\xF8\\x${this.#format_byte(index)}`;
   }
   
   // F9 xx or FC 0C xx
   handle_extra_symbol(index, escape_type) {
      let cc = this.#format_byte(index);
      if (escape_type == 0xF9)
         this.result += `\\xF9\\x${cc}`;
      else
         this.result += `\\xFC\\x0C\\x${cc}`;
   }
   
   // FC 00
   handle_ctrl_location_end() {
      this.result += `\\xFC\\x00`;
   }
   
   // FC xx yy
   //  - xx == 01: color
   //  - xx == 02: highlight
   //  - xx == 03: shadow
   //  - xx == 05: palette
   handle_ctrl_set_color(type, index) {
      let tb = { color: 0x01, highlight: 0x02, shadow: 0x03, palette: 0x05 };
      this.result += `\\xFC\\x${this.#format_byte(tb)}\\x${this.#format_byte(index)}`;
   }
   
   // Also called repeatedly for FC 04 xx yy zz given color, highlight, shadow
   handle_ctrl_set_all_colors(color, highlight, shadow) {
      this.result += `\\xFC\\x04\\x${this.#format_byte(color)}\\x${this.#format_byte(highlight)}\\x${this.#format_byte(shadow)}`;
   }
   
   // FC 06 xx
   handle_ctrl_set_font(index) {
      this.result += `\\xFC\\x06\\x${this.#format_byte(index)}`;
   }
   
   // FC 07
   handle_ctrl_reset_font() {
      this.result += `\\xFC\\x07`;
   }
   
   // FC 08 xx
   handle_ctrl_pause(delay) {
      this.result += `\\xFC\\x08\\x${this.#format_byte(delay)}`;
   }
   
   // FC 09
   handle_ctrl_pause_until_press() {
      this.result += `\\xFC\\x09`;
   }
   
   // FC 0A
   handle_ctrl_wait_for_sound() {
      this.result += `\\xFC\\x0A`;
   }
   
   // FC 0B xx xx
   handle_ctrl_play_music(index) {
      this.result += `\\xFC\\x0B${this.#format_word(index)}`;
   }
   
   // FC 0D xx
   // FC 0E yy
   handle_ctrl_shift(axis, by) {
      let a = axis == "x" ? 0x0D : 0x0E;
      this.result += `\\xFC\\x${this.#format_byte(a)}\\x${this.#format_byte(by)}`;
   }
   
   // FC 0F
   handle_ctrl_fill_window() {
      this.result += `\\xFC\\x0F`;
   }
   
   // FC 10 xx xx
   handle_ctrl_play_sound(index) {
      this.result += `\\xFC\\x10${this.#format_word(index)}`;
   }
   
   // FC 11 xx
   // Only useful in combination with "shift right," to "rewind" the text and 
   // print something else?
   handle_ctrl_clear_pixels_ahead(by) {
      this.result += `\\xFC\\x11\\x${this.#format_byte(by)}`;
   }
   
   // FC 12 xx
   handle_ctrl_skip_pixels(x) {
      this.result += `\\xFC\\x12\\x${this.#format_byte(x)}`;
   }
   
   // FC 13 xx
   // Clear pixels up to some X-coordinate, on the current line. The coordinate 
   // is relative to the X-position of the region we're printing to.
   handle_ctrl_clear_to_x_coordinate(x) {
      this.result += `\\xFC\\x13\\x${this.#format_byte(x)}`;
   }
   
   // FC 14 xx
   // Force characters to have a minimum width. Characters are not centered within 
   // their "cells." Characters wider than a "cell" are not clipped.
   handle_ctrl_set_letter_spacing(v) {
      this.result += `\\xFC\\x14\\x${this.#format_byte(v)}`;
   }
   
   // FC 15 for "japanese"
   // FC 16 for "latin"
   handle_ctrl_set_character_set(what) {
      this.result += `\\xFD\\x${this.#format_byte(what == "japanese" ? 0x15 : 0x16)}`;
      this.#current_charset = what;
   }
   
   // FC 17
   handle_ctrl_pause_music() {
      this.result += `\\xFC\\x17`;
   }
   
   // FC 18
   handle_ctrl_resume_music() {
      this.result += `\\xFC\\x18`;
   }
   
   // FD xx
   handle_string_substitution(index) {
      this.result += `\\xFD\\x${this.#format_byte(index)}`;
   }
};