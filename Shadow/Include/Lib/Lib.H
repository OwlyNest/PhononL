/*
	* Shadow/Include/Lib/Lib.H - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 21 11:55:51 2026
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
#ifndef __LIB_LIB_H__
#define __LIB_LIB_H__

/* --- Includes ---*/
#include <Lib/Math.H>
#include <Lib/PrintK.H>
#include <stdarg.h>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

#define XAL_EMIT_PROTOTYPES
#include <XAL/xSCAL.H>
#include <Lib/Lib.xal>
#undef XAL_EMIT_PROTOTYPES
#undef XAL_METHOD

#endif /* __LIB_LIB_H__ */