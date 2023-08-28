#pragma once
#include "../common/types.h"

void lu_PrepStringWrap_Normal(void);

// Call after configuring your printer and expanding string placeholders. It will scan 
// over the expanded string, computing word/line lengths and replacing spaces with a 
// newline or press-A-to-scroll character code as appropriate.
u8* lu_StringWrap(u8* str);