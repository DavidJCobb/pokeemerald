#include "global.h"
#include "text.h"
#include "window.h"
#include "fonts.h"

// duplicated from vanilla text.c
struct {
   u16 tileOffset;
   u8  width;
   u8  height;
} static const sKeypadIcons[] = {
    [CHAR_A_BUTTON]       = { 0x00,  8, 12 },
    [CHAR_B_BUTTON]       = { 0x01,  8, 12 },
    [CHAR_L_BUTTON]       = { 0x02, 16, 12 },
    [CHAR_R_BUTTON]       = { 0x04, 16, 12 },
    [CHAR_START_BUTTON]   = { 0x06, 24, 12 },
    [CHAR_SELECT_BUTTON]  = { 0x09, 24, 12 },
    [CHAR_DPAD_UP]        = { 0x0C,  8, 12 },
    [CHAR_DPAD_DOWN]      = { 0x0D,  8, 12 },
    [CHAR_DPAD_LEFT]      = { 0x0E,  8, 12 },
    [CHAR_DPAD_RIGHT]     = { 0x0F,  8, 12 },
    [CHAR_DPAD_UPDOWN]    = { 0x20,  8, 12 },
    [CHAR_DPAD_LEFTRIGHT] = { 0x21,  8, 12 },
    [CHAR_DPAD_NONE]      = { 0x22,  8, 12 }
};
// end

static struct SimulatedTextPrinting {
   
   // window-relative offset, not absolute.
   //
   // this must be a u16 because we check if a word is too long and needs to be wrapped 
   // only once we reach the end of the word. the screen is 256px long, and the typical 
   // window for drawing dialogue has an inner size (not including some reserved space) 
   // of 200px. a sufficiently long word right at the very, very end of the available 
   // space will overflow a u8.
   u16 current_x;
   //
   u8  word_width;
   u8  word_length;
   u8  line_count; // used so we know if we need CHAR_PROMPT_SCROLl or CHAR_NEWLINE to wrap
   
   u8  canvas_width;
   u8  canvas_lines; // number of lines of text that the canvas is tall enough to display
   u8  font_id;
   u8  letter_spacing;
   u8  override_spacing; // when non-zero, forces monospacing
   u8  japanese;
   
   u8* last_seen_space; // last-seen space character that we are allowed to overwrite with '\n' or '\p'
   u16 last_space_x;    // righthand edge of last seen space
} _sim_state;

void lu_PrepStringWrap(
   u8 win_id,
   u8 font_id
) {
   _sim_state.japanese = FALSE;
   _sim_state.override_spacing = 0;
   
   _sim_state.font_id = font_id;
   _sim_state.letter_spacing = GetFontAttribute(font_id, FONTATTR_LETTER_SPACING);
   
   _sim_state.canvas_width = gWindows[win_id].window.width * 8; // GBA tiles -> pixels
   _sim_state.canvas_width -= 16; // Reserve space for the down-arrow used to prompt the player to press A to scroll, plus roughly 8px padding.
   _sim_state.canvas_lines = 1;
   {
      u8 win_height  = gWindows[win_id].window.height * 8; // GBA tiles -> pixels
      u8 text_height = gFonts[_sim_state.font_id].maxLetterHeight;
      while (text_height < win_height) {
         ++_sim_state.canvas_lines;
         text_height += gFonts[_sim_state.font_id].maxLetterHeight + GetFontAttribute(font_id, FONTATTR_LINE_SPACING);
      }
   }
}

void lu_PrepStringWrap_Normal(void) {
   lu_PrepStringWrap(0, FONT_NORMAL);
}

static void lu_StringWrap_OnSpace(u8 current_char, u8* str) {
   if (_sim_state.current_x > _sim_state.canvas_width) {
      // TODO: if _sim_state.word_length >= 6, consider breaking and hyphenating
      //       (...but if we're overwriting the string in place, how do we do that?
      //       we'd have to add a 1-byte control code so "\n-" can be just one char.)
      
      if (_sim_state.last_seen_space != NULL) {
         *_sim_state.last_seen_space = CHAR_NEWLINE;
         ++_sim_state.line_count;
         if (_sim_state.line_count >= _sim_state.canvas_lines) {
            *_sim_state.last_seen_space = CHAR_PROMPT_SCROLL;
         }
         _sim_state.current_x -= _sim_state.last_space_x;
      }
   }
   if (current_char == CHAR_SPACE) {
      _sim_state.last_seen_space = str - 1;
   } else {
      _sim_state.last_seen_space = NULL;
   }
   _sim_state.last_space_x = _sim_state.current_x;
   _sim_state.word_length = 0;
   _sim_state.word_width  = 0;
}

// Based on StringExpandPlaceholders in gflib/string_util.c
u8* lu_StringWrap(u8* str) {
   _sim_state.current_x   = 0;
   _sim_state.word_width  = 0;
   _sim_state.word_length = 0;
   _sim_state.line_count  = 0;
   
   while (TRUE) {
      u8 glyph_width = 0;
      
      u8  current_byte = *str++;
      u16 current_char = current_byte;
      
      switch (current_byte) {
         case CHAR_NEWLINE:
            lu_StringWrap_OnSpace(current_byte, str);
            ++_sim_state.line_count;
            _sim_state.current_x   = 0;
            _sim_state.word_length = 0;
            _sim_state.word_width  = 0;
            _sim_state.last_seen_space = NULL;
            _sim_state.last_space_x    = 0;
            continue;
            
         case CHAR_PROMPT_CLEAR:
            lu_StringWrap_OnSpace(current_byte, str);
            _sim_state.line_count  = 0;
            _sim_state.current_x   = 0;
            _sim_state.word_length = 0;
            _sim_state.word_width  = 0;
            _sim_state.last_seen_space = NULL;
            _sim_state.last_space_x    = 0;
            continue;
            
         case CHAR_PROMPT_SCROLL:
            lu_StringWrap_OnSpace(current_byte, str);
            _sim_state.line_count  = _sim_state.canvas_lines;
            _sim_state.word_length = 0;
            _sim_state.word_width  = 0;
            _sim_state.last_seen_space = NULL;
            _sim_state.last_space_x    = 0;
            continue;
         
         case CHAR_EXTRA_SYMBOL:
            current_char = *str++ | 0x100;
            break;
         
         case CHAR_KEYPAD_ICON:
            current_byte = *str++;
            _sim_state.current_x += sKeypadIcons[current_byte].width + _sim_state.letter_spacing;
            continue;
         
         case EOS:
            lu_StringWrap_OnSpace(current_byte, str);
            break;
         
         // should never happen
         case PLACEHOLDER_BEGIN:
            continue;
         
         case EXT_CTRL_CODE_BEGIN:
            
            // For this nested switch-case, `continue` if the control code sequence does not 
            // produce a (potentially) printable character.
            
            current_byte = *str++;
            switch (current_byte) {
               case EXT_CTRL_CODE_FONT:
                  _sim_state.font_id = *str++;
                  continue;
               case EXT_CTRL_CODE_RESET_FONT:
                  //_sim_state.font_id = printer_substruct->fontId; // TODO: Is this control code functional?
                  continue;
               case EXT_CTRL_CODE_ESCAPE:
                  current_char = *str++ | 0x100;
                  break;
               case EXT_CTRL_CODE_SHIFT_RIGHT:
               case EXT_CTRL_CODE_SKIP:
                  _sim_state.last_seen_space = NULL;
                  _sim_state.last_space_x    = 0;
                  _sim_state.current_x   = *str++;
                  _sim_state.word_width  = 0;
                  _sim_state.word_length = 0;
                  continue;
               case EXT_CTRL_CODE_FILL_WINDOW:
                  _sim_state.last_seen_space = NULL;
                  _sim_state.last_space_x    = 0;
                  _sim_state.current_x   = 0;
                  _sim_state.line_count  = 0;
                  _sim_state.word_width  = 0;
                  _sim_state.word_length = 0;
                  continue;
               case EXT_CTRL_CODE_CLEAR:
                  _sim_state.last_seen_space = str;
                  _sim_state.last_seen_space--;
                  _sim_state.word_width  = 0;
                  _sim_state.word_length = 0;
                  //
                  current_byte = *str++;
                  if (current_byte > 0) {
                     _sim_state.current_x += current_byte;
                  }
                  _sim_state.last_space_x = _sim_state.current_x;
                  continue;
               case EXT_CTRL_CODE_MIN_LETTER_SPACING:
                  _sim_state.override_spacing = *str++;
                  continue;
               case EXT_CTRL_CODE_JPN:
                  _sim_state.japanese = TRUE;
                  continue;
               case EXT_CTRL_CODE_ENG:
                  _sim_state.japanese = FALSE;
                  continue;
                  
               // Control codes we don't care about:
                  
               case EXT_CTRL_CODE_COLOR_HIGHLIGHT_SHADOW: // Control code consumes 3 more bytes...
                  ++str;
                  // fallthrough
               case EXT_CTRL_CODE_PLAY_BGM: // Control code consumes 2 more bytes...
               case EXT_CTRL_CODE_PLAY_SE:
                  ++str;
                  // fallthrough
               case EXT_CTRL_CODE_COLOR: // Control code consumes 1 more byte...
               case EXT_CTRL_CODE_HIGHLIGHT:
               case EXT_CTRL_CODE_SHADOW:
               case EXT_CTRL_CODE_PALETTE:
               case EXT_CTRL_CODE_SHIFT_DOWN:
               case EXT_CTRL_CODE_PAUSE:
               case EXT_CTRL_CODE_CLEAR_TO:
                  ++str;
                  // fallthrough
               case EXT_CTRL_CODE_PAUSE_UNTIL_PRESS: // Control code consumes no more bytes...
               case EXT_CTRL_CODE_WAIT_SE:
               case EXT_CTRL_CODE_PAUSE_MUSIC:
               case EXT_CTRL_CODE_RESUME_MUSIC:
                  continue;
            }
      }
      
      if (current_byte == EOS) {
         break;
      }
      
      if (_sim_state.japanese) {
         //
         // All Japanese fonts except the "short" fonts use 8-pixel character widths.
         //
         glyph_width = 8;
         switch (_sim_state.font_id) {
            case FONT_SHORT:
            case FONT_SHORT_COPY_1:
            case FONT_SHORT_COPY_2:
            case FONT_SHORT_COPY_3:
               glyph_width = gFontShortJapaneseGlyphWidths[current_char];
               break;
            case FONT_BRAILLE:
               // TODO: Braille isn't handled by the normal RenderText function, but... should 
               //       *we* handle it here?
               glyph_width = 0;
               break;
         }
      } else {
         switch (_sim_state.font_id) {
            case FONT_SMALL:
               glyph_width  = gFontSmallLatinGlyphWidths[current_char];
               break;
            case FONT_NORMAL:
               glyph_width  = gFontNormalLatinGlyphWidths[current_char];
               break;
            case FONT_SHORT:
            case FONT_SHORT_COPY_1:
            case FONT_SHORT_COPY_2:
            case FONT_SHORT_COPY_3:
               glyph_width = gFontShortLatinGlyphWidths[current_char];
               break;
            case FONT_NARROW:
               gFontNarrowLatinGlyphWidths[current_char];
               break;
            case FONT_SMALL_NARROW:
               gFontSmallNarrowLatinGlyphWidths[current_char];
               break;
            case FONT_BRAILLE:
               // TODO: Braille isn't handled by the normal RenderText function, but... should 
               //       *we* handle it here?
               break;
         }
      }
      
      if (_sim_state.override_spacing) {
         if (_sim_state.override_spacing > glyph_width) {
            _sim_state.current_x += _sim_state.override_spacing;
         } else {
            _sim_state.current_x += glyph_width;
         }
      } else {
         if (_sim_state.japanese)
            _sim_state.current_x += glyph_width + _sim_state.letter_spacing;
         else
            _sim_state.current_x += glyph_width;
      }
      
      //
      // Check if we need to break the text:
      //
      if (current_char == CHAR_SPACE) {
         lu_StringWrap_OnSpace(current_char, str);
      } else {
         ++_sim_state.word_length;
         _sim_state.word_width += glyph_width;
      }
   }
   return str;
}