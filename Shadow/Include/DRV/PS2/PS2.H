/*
	* Shadow/Include/DRV/PS2/PS2.H - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 28 02:14:54 2026
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
#ifndef __DRV_PS2_PS2_H__
#define __DRV_PS2_PS2_H__

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/
/*
	* A raw key event, deliberately nothing more. Scancode is the Set 1
	* make code with the release bit stripped (the controller translates
	* Set 2 to Set 1, see I8042.C). Layouts, modifiers and repeat are a
	* later layer's problem -- no keymap table belongs in the driver.
*/
typedef struct PS2_KEY {
	UINT8   Scancode;
	BOOLEAN Pressed;    /* FALSE = release */
	BOOLEAN Extended;   /* was preceded by the 0xE0 prefix */
} _PS2_KEY, *_PPS2_KEY;

/* --- Globals ---*/

/* --- Prototypes ---*/
#define XAL_EMIT_PROTOTYPES
#include <XAL/xSCAL.H>
#include <DRV/PS2/PS2.XAL>
#undef XAL_EMIT_PROTOTYPES
#undef XAL_METHOD

#endif /* __DRV_PS2_PS2_H__ */