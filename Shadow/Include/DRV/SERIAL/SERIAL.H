/*
	* Shadow/Include/DRV/SERIAL/SERIAL.H - Serial (16550 UART) output
	* Author:   amity
	* Date:     Fri Sep 25 00:40:38 2026
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
#ifndef __DRV_SERIAL_SERIAL_H__
#define __DRV_SERIAL_SERIAL_H__

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
#define XAL_EMIT_PROTOTYPES
#include <XAL/xSCAL.H>
#include <DRV/SERIAL/SERIAL.XAL>
#undef XAL_EMIT_PROTOTYPES
#undef XAL_METHOD

#endif /* __DRV_SERIAL_SERIAL_H__ */