/*
	* Shadow/GAL/GAL.C - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 14 12:40:04 2026
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
#include <GAL/GAL.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static _GAL_BACKEND *ActiveBackend = NULL;

/* --- Prototypes ---*/

/* --- Functions ---*/
#define XAL_PREFIX GAL
#define XAL_BACKEND _PGAL_BACKEND
#define XAL_EMIT_DISPATCH
#include <XAL/xMCAL.H>
#include <GAL/GAL.xal>
#undef XAL_EMIT_DISPATCH
#undef XAL_METHOD
#undef XAL_METHOD_VOID
#undef XAL_PREFIX
#undef XAL_BACKEND