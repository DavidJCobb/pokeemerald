
//
// TODO: Replace this with Charmap
//

class Charset {
   constructor() {
      this.bytes_to_chars = {};
      this.chars_to_bytes = {};
      
      this.byte_sequences = {}; // [name] == [byte, byte]; if `byte` == "*", then matches any byte
      
      this.entities = {}; // [name] == [byte, byte, ...] // stores canonical names
      this.entities_lower = {}; // stores lowercase names, for case-insensitive lookups
   }
   
   run(base, str) {
      for(let i = 0; i < str.length; ++i) {
         let cc = base + i;
         let ch = str[i];
         this.bytes_to_chars[cc] = ch;
         this.chars_to_bytes[ch] = cc;
      }
   }
   define_char_code(code, char) {
      this.bytes_to_chars[code] = char;
      this.chars_to_bytes[char] = code;
   }
   
   define_byte_sequence(name, ...bytes) {
      this.byte_sequences[name] = [...bytes];
   }
   
   define_entity(name, bytes) {
      this.entities[name] = bytes;
      this.entities_lower[name.toLowerCase()] = bytes;
   }
   get_entity(name) {
      return this.entities_lower[name.toLowerCase()];
   }
};

const CHARSET_CONTROL_CODES = (function() {
   let out = new Charset();
   for(let [i, name] of [
      "A", "B", "L", "R", "Start", "Select", "DPadUp", "DPadDown", "DPadLeft", "DPadRight", "DPadUpDown", "DPadLeftRight", "DPadNone"
   ]) {
      let en = "Button" + name;
      out.define_entity(en, [0xF8, i]);
   }
   //
   out.define_entity("LocationNameEndMarker", [0xFC, 0x00]);
   out.define_entity("FontSmall", [0xFC, 0x06, 0x00]);
   out.define_entity("FontNormal", [0xFC, 0x06, 0x01]);
   out.define_entity("FontShort", [0xFC, 0x06, 0x02]);
   out.define_entity("FontNarrow", [0xFC, 0x06, 0x07]);
   out.define_entity("FontSmallNarrow", [0xFC, 0x06, 0x08]);
   out.define_entity("ResetFont", [0xFC, 0x07]);
   out.define_entity("ToJapanese", [0xFC, 0x15]);
   out.define_entity("ToLatin", [0xFC, 0x16]);
   out.define_entity("PauseMusic", [0xFC, 0x17]);
   out.define_entity("ResumeMusic", [0xFC, 0x18]);
   //
   out.define_entity("PlayerName", [0xFD, 0x01]);
   out.define_entity("StrVar1", [0xFD, 0x02]);
   out.define_entity("StrVar2", [0xFD, 0x03]);
   out.define_entity("StrVar3", [0xFD, 0x04]);
   out.define_entity("HonorificChild", [0xFD, 0x05]);
   out.define_entity("RivalName", [0xFD, 0x06]);
   out.define_entity("GameVersion", [0xFD, 0x07]);
   out.define_entity("TeamAquaName", [0xFD, 0x08]);
   out.define_entity("TeamMagmaName", [0xFD, 0x09]);
   out.define_entity("TeamAquaLeaderName", [0xFD, 0x0A]);
   out.define_entity("TeamMagmaLeaderName", [0xFD, 0x0B]);
   out.define_entity("TeamAquaLegendaryName", [0xFD, 0x0C]);
   out.define_entity("TeamMagmaLegendaryName", [0xFD, 0x0D]);
   
   out.define_char_code(0xF7, "DYNAMIC");
   out.define_byte_sequence("NAME_END", 0xFC, 0x00);
   out.define_byte_sequence("COLOR", 0xFC, 0x01, "*");
   out.define_byte_sequence("HIGHLIGHT", 0xFC, 0x02, "*");
   out.define_byte_sequence("SHADOW", 0xFC, 0x03, "*");
   out.define_byte_sequence("COLOR_HIGHLIGHT_SHADOW", 0xFC, 0x041, "*", "*", "*");
   out.define_byte_sequence("PALETTE", 0xFC, 0x05, "*");
   out.define_byte_sequence("FONT", 0xFC, 0x06, "*");
   out.define_byte_sequence("RESET_FONT", 0xFC, 0x07);
   out.define_byte_sequence("PAUSE", 0xFC, 0x08, "*");
   out.define_byte_sequence("PAUSE_UNTIL_PRESS", 0xFC, 0x09);
   out.define_byte_sequence("WAIT_SE", 0xFC, 0x0A);
   out.define_byte_sequence("PLAY_BGM", 0xFC, 0x0B, "*", "*");
   out.define_byte_sequence("STRING_EXPANSION", 0xFD, "*");
   out.define_byte_sequence("ESCAPE", 0xFC, 0x0C, "*"); // guessed
   out.define_byte_sequence("SHIFT_RIGHT", 0xFC, 0x0D, "*"); // guessed
   out.define_byte_sequence("SHIFT_DOWN", 0xFC, 0x0D, "*"); // guessed
   // FILL_WINDOW
   out.define_byte_sequence("PLAY_SE", 0xFC, 0x10, "*", "*");
   // CLEAR
   // SKIP
   // CLEAR_TO
   // MIN_LETTER_SPACING
   // JPN
   // ENG
   // PAUSE_MUSIC
   // RESUME_MUSIC
   for(let [i, name] of [
      "A_BUTTON",
      "B_BUTTON",
      "L_BUTTON",
      "R_BUTTON",
      "START_BUTTON",
      "SELECT_BUTTON",
      "DPAD_UP",
      "DPAD_DOWN",
      "DPAD_LEFT",
      "DPAD_RIGHT",
      "DPAD_UPDOWN",
      "DPAD_LEFTRIGHT",
      "DPAD_NONE",
   ]) {
      out.define_byte_sequence(name, 0xF8, i);
   }
   out.define_byte_sequence("INVALID_BUTTON_ICON", 0xF8, "*");
   
   out.define_byte_sequence("EMOJI", 0xF9, "*");
   out.define_char_code(0xFA, "\\l");
   out.define_char_code(0xFB, "\\p");
   out.define_char_code(0xFE, "\\n");
   out.define_char_code(0xFF, "\0");
   return out;
})();

const CHARSET_ENGLISH = (function() {
   let out = new Charset();
   
   out.run(0x00, " ÀÁÂÇÈÉÊËÌÎÏÒÓÔŒÙÚÛÑßàá");
   out.run(0x19, "çèéêëì");
   out.run(0x20, "îïòóôœùúûñºª");
   out.define_char_code(0x2C, "SUPER_ER");
   out.run(0x2D, "&+");
   out.define_char_code(0x34, "LV"); // "Lv." glyph
   out.define_char_code(0x35, "=");
   out.define_char_code(0x36, ";");
   out.define_char_code(0x51, "¿");
   out.define_char_code(0x52, "¡");
   out.define_char_code(0x53, "PKMN_PK");
   out.define_char_code(0x54, "PKMN_MN");
   out.define_char_code(0x55, "POKEBLOCK_PO");
   out.define_char_code(0x56, "POKEBLOCK_KE");
   out.define_char_code(0x57, "POKEBLOCK_BL");
   out.define_char_code(0x58, "POKEBLOCK_OC");
   out.define_char_code(0x59, "POKEBLOCK_K");
   out.run(0x5A, "Í%()");
   out.define_char_code(0x68, "â");
   out.define_char_code(0x6F, "í");
   out.define_char_code(0x77, "UNK_SPACER");
   out.define_char_code(0x79, "UP_ARROW");
   out.define_char_code(0x7A, "DOWN_ARROW");
   out.define_char_code(0x7B, "LEFT_ARROW");
   out.define_char_code(0x7C, "RIGHT_ARROW");
   out.define_char_code(0x84, "SUPER_E");
   out.run(0x85, "<>");
   out.define_char_code(0xA0, "SUPER_RE");
   out.run(0xA1, "0123456789!?.-·…“”‘’♂♀¥,×/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz▶:ÄÖÜäöü");
   out.define_byte_sequence("TALL_PLUS", 0xFC, 0x0C, 0xFB);
   
   return out;
})();
const CHARSET = CHARSET_ENGLISH;