#include "global.h"

// Use the appropriate "prep" function given where and what text you're going to display.

void lu_PrepStringWrap(u8 windowId, u8 fontId);

// window 0, FONT_NORMAL
void lu_PrepStringWrap_Normal(void);

// Call after configuring your printer and expanding string placeholders. It will scan 
// over the expanded string, computing word/line lengths and replacing spaces with a 
// newline or press-A-to-scroll character code as appropriate.
u8* lu_StringWrap(u8* str);