/*
	* Shadow/Include/GFX/Console.H - simple scrolling text console for printk
	* Author:   amity
	* Date:     Tue Sep 15 00:20:36 2026
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
#ifndef __GFX_CONSOLE_H__
#define __GFX_CONSOLE_H__

#define CONSOLE_DEFAULT_COLS      80
#define CONSOLE_DEFAULT_ROWS      40
#define CHAR_W                    	8
#define CHAR_H                    8
#define CONSOLE_FG                GfxThemeColor(GFX_FG_TEXT)
#define CONSOLE_BG                GfxThemeColor(GFX_BG_DESKTOP)

#define CONSOLE_DEFAULT_TAB_WIDTH 4

/* --- Includes ---*/
#include <GFX/FB.H>
#include <Lib/Lib.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
VOID ConsoleInit(VOID);
VOID ConsoleClear(VOID);
VOID ConsoleWrite(PCCHAR str);
VOID ConsoleRedraw(VOID);
INT  ConsoleIsDirty(VOID);

#endif /* __GFX_CONSOLE_H__ */