/*
	* Shadow/GFX/Console.C - simple scrolling text console for printk
	* Author:   amity
	* Date:     Tue Sep 15 00:20:30 2026
	* Copyright © 2026 OwlyNest
*/

/* --- Styling Instructions ---
	* Encoding:                      UTF-8, Unix line endings
	* Text font:                     Monospace
	* Line width:                    Max 80 characters
	* Indentation:                   Use 4 spaces
	* Brace style:                   Same line as control statement
	* Inline comments:               Column 40, wherever possible, else, whole multiple of 20
	* Section headers:               Use 3 '-' characters before and after
	* Pointer notation:              Next to variable name, not type
	* Binary operations:             Space around operator
	* Empty parameter list:          Use (void) instead of ()
	* Statements and declarations:   Max one per line
*/

/* --- Macros ---*/

/* --- Includes ---*/
#include <GFX/GFX.H>
#include <Lib/Lib.H>
#include <GFX/Console.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static CHAR  ConsoleBuf[CONSOLE_ROWS][CONSOLE_COLS + 1];
static INT   ConsoleRow   =  0;
static INT   ConsoleCol   =  0;
static INT   ConsoleReady =  0;
/* Dirty range (inclusive) -1 means "nothing" */
static INT   DirtyTop     = -1;	
static INT   DirtyBottom  = -1;

/* --- Prototypes ---*/

/* --- Functions ---*/
static VOID MarkDirty(
	IN INT Row
) {
	if (Row < 0 || Row >= CONSOLE_ROWS) {
		return;
	}

	if (DirtyTop < 0) {
		DirtyTop = DirtyBottom = Row;
	} else {
		if (Row < DirtyTop) {
			DirtyTop = Row;
		}
		if (Row > DirtyBottom) {
			DirtyBottom = Row;
		}
	}
}

static VOID MarkAllDirty(VOID) {
	DirtyTop    = 0;
	DirtyBottom = CONSOLE_ROWS - 1;
}

static VOID ConsoleScroll(VOID) {
	for (INT r = 0; r < CONSOLE_ROWS - 1; r++) {
		MemCpy(ConsoleBuf[r], ConsoleBuf[r + 1], CONSOLE_COLS + 1);
	}

	MemSet(ConsoleBuf[CONSOLE_ROWS - 1], 0, CONSOLE_COLS - 1);
	if (ConsoleRow > 0) {
		ConsoleRow--;
	}

	MarkAllDirty();
}

static VOID ConsoleLineFeed(VOID) {
	/*
	 * Oh no, we are not doing the LF Linux thing, Why even HAVE CR
	 * And besides, I don't like an OS that circumvents my hardware
	 * And for what? One character less?
	*/
	ConsoleRow++;
	if (ConsoleRow >= CONSOLE_ROWS) {
		ConsoleScroll();
		ConsoleRow = CONSOLE_ROWS - 1;
	} else {
		MarkDirty(ConsoleRow);
	}
}

static VOID ConsoleCarriageReturn(VOID) {
	ConsoleCol = 0;
	MarkDirty(ConsoleRow);
}

static VOID ConsoleTab(VOID) {
	ConsoleCol = (ConsoleCol + CONSOLE_DEFAULT_TAB_WIDTH) & ~(CONSOLE_DEFAULT_TAB_WIDTH - 1);
	if (ConsoleCol >= CONSOLE_COLS) {
		ConsoleCarriageReturn();
		ConsoleLineFeed();
	} else {
		MarkDirty(ConsoleRow);
	}
	return;
}

static VOID ConsolePutc(CHAR C) {
	if (C == '\n') {
		ConsoleLineFeed();
		return;
	}

	if (C == '\r') {
		ConsoleCarriageReturn();
		return;
	}

	if (C == '\t') {
		ConsoleTab();
		return; /* Oopsie 🦄 */
	}

	if (ConsoleCol >= CONSOLE_COLS) {
		ConsoleCarriageReturn();
		ConsoleLineFeed();
	}

	ConsoleBuf[ConsoleRow][ConsoleCol++] = C;
	ConsoleBuf[ConsoleRow][ConsoleCol] = '\0';
	MarkDirty(ConsoleRow);	
}

VOID ConsoleInit(VOID) {
	MemSet(ConsoleBuf, 0, sizeof(ConsoleBuf));
	ConsoleRow   = 0;
	ConsoleCol 	 = 0;
	/*
	 * Dirty & Ready... sounds like me, heh!
	*/
	// ConsoleDirty = 1;
	ConsoleReady = 1;
	MarkAllDirty(); /* Has less of a ring to it */
}

VOID ConsoleClear(VOID) {
	/*
	 * See, Kitty devs, It's not that hard
	*/

	MemSet(ConsoleBuf, 0, sizeof(ConsoleBuf));
	ConsoleRow = 0;
	ConsoleCol = 0;
	MarkAllDirty();
}

VOID ConsoleWrite(
	IN PCCHAR Str
) {
	if (!ConsoleReady || !Str) {
		return;
	}

	while (*Str) {
		ConsolePutc(*Str++); /* Girls having trouble finding the *-spot */
	}
}

/*
 * Only touch the dirty row range.
 * For each dirty line we:
 *   1. fill the 8-pixel-high strip with background
 *   2. draw the string
 * That is dramatically cheaper than a full-screen clear on real hardware.
*/
VOID ConsoleRedraw(VOID) {
    if (!ConsoleReady || !fb.Initialized)
        return;
    if (DirtyTop < 0)
        return;                     /* nothing to do */

    for (INT r = DirtyTop; r <= DirtyBottom; r++) {
        UINT32 y = (UINT32)(r * CHAR_H);

        /* clear just this character row */
        GfxFillRect(&fb.Back, 0, y, fb.Back.Width, CHAR_H, CONSOLE_BG);

        if (ConsoleBuf[r][0] != '\0') {
            GfxDrawString(&fb.Back, 0, y, ConsoleBuf[r], CONSOLE_FG);
        }
    }

    DirtyTop    = -1;
    DirtyBottom = -1;
}

INT ConsoleIsDirty(VOID) {
	return DirtyTop >= 0;
}