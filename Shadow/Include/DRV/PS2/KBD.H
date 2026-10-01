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
#define PS2_KBD_CMD_SET_LEDS      0xED /* + LED byte, ACKed twice  */
#define PS2_KBD_CMD_ECHO          0xEE /* Diagnostic; removal probe */
#define PS2_KBD_CMD_SET_SCANSET   0xF0 /* Footgun: see comment      */
#define PS2_KBD_CMD_SET_TYPEMATIC 0xF3 /* + rate/delay byte         */
#define PS2_KBD_CMD_DEFAULTS      0xF6 /* Restore default params    */

/* Set 3 only: meaningless under translation. Kept for completeness;
	do not send. */
#define PS2_KBD_CMD_ALL_TYPEMATIC 0xF7
#define PS2_KBD_CMD_ALL_MAKE_REL  0xF8
#define PS2_KBD_CMD_ALL_MAKE      0xF9
#define PS2_KBD_CMD_ALL_TMR       0xFA
#define PS2_KBD_CMD_KEY_TYPEMATIC 0xFB /* + scancode */
#define PS2_KBD_CMD_KEY_MAKE_REL  0xFC /* + scancode */
#define PS2_KBD_CMD_KEY_MAKE      0xFD /* + scancode */

/* Same value as PS2_DEV_RESEND, opposite direction: the host asking
	the device to repeat its last byte. */
#define PS2_KBD_CMD_RESEND        0xFE

/* --- LED state byte (argument to 0xED) ---*/
#define PS2_KBD_LED_SCROLL_LOCK (1 << 0)
#define PS2_KBD_LED_NUM_LOCK    (1 << 1)
#define PS2_KBD_LED_CAPS_LOCK   (1 << 2)

/* --- 0xF0 sub-commands and its translated responses ---*/
#define PS2_KBD_SCANSET_GET    0x00
#define PS2_KBD_SCANSET_1      0x01
#define PS2_KBD_SCANSET_2      0x02
#define PS2_KBD_SCANSET_3      0x03
/* Note 0x41: the same byte as the MF2 keyboard's identify response.
   "Translated Set 2" all the way down. */
#define PS2_KBD_SCANSET_RESP_1 0x43
#define PS2_KBD_SCANSET_RESP_2 0x41
#define PS2_KBD_SCANSET_RESP_3 0x3F

/* --- 0xF3 typematic byte fields ---*/
#define PS2_KBD_TYPEMATIC_RATE  0x1F /* bits 0-4: 30 Hz .. 2 Hz */
#define PS2_KBD_TYPEMATIC_DELAY 0x60 /* bits 5-6: 250..1000 ms  */

/* --- Special bytes the keyboard sends (never scancodes) ---*/
#define PS2_KBD_ERR_OVERRUN0 0x00 /* Key detection error / overrun */
#define PS2_KBD_ERR_OVERRUN1 0xFF /* ditto                         */

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

#endif /* __DRV_PS2_KBD_H__ */