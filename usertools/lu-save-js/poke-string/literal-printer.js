import CHARMAP from "./charmap.js";
import AbstractPokeStringPrinter from "./printer.js";

export default class LiteralPokeStringPrinter extends AbstractPokeStringPrinter {
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