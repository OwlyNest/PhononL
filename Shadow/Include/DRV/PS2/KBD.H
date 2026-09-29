/*
	* Shadow/Include/DRV/PS2/KBD.H - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 28 02:12:11 2026
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
#ifndef __DRV_PS2_KBD_H__
#define __DRV_PS2_KBD_H__

/*
	* Ring depth. Power of two. One slot is ALWAYS left empty, so that
	* Head == Tail unambiguously means "empty", the classic full/empty
	* disambiguation, no counter, no second flag. Depth is a private
	* implementation choice: nobody outside KBD.C contracts over it, so
	* it lives here, not in KBD.H.
*/
#define KBD_RING_SIZE 32

/* --- Set 1 scancode vocabulary ---
	* Set 1 is what the CPU sees with the controller's translation on
	* (config bit 6, armed by I8042 init). Breaks are the SAME byte
	* with bit 7 set, not a prefix. (Set 2, which the keyboard
	* physically clocks out, uses an 0xF0 break prefix -- translation
	* is what hides it.)
*/
#define PS2_SET1_EXTENDED      0xE0 /* Prefix: next byte is extended  */
#define PS2_SET1_PAUSE         0xE1 /* Prefix: 6-byte seq, press-only */
#define PS2_SET1_BREAK         0x80 /* BIT: set = release             */
#define PS2_SET1_SYSRQ_FIRST   0x2A /* E0 2A ...: Print Screen half   */
#define PS2_SET1_SYSRQ_ID      0x37 /* Synthetic scancode for SysRq   */
#define PS2_SET1_PAUSE_LEN     6    /* E1 + 5 swallowed bytes         */

/* --- Keyboard device commands (KBD.H: keyboard-only protocol) ---*/
#define PS2_KBD_CMD_ENABLE_SCAN  0xF4 /* Start sending scancodes */
#define PS2_KBD_CMD_DISABLE_SCAN 0xF5 /* Stop scanning           */

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

#endif /* __DRV_PS2_KBD_H__ */