/*
	* Shadow/TAL/TAL.C - Timer Abstraction Layer: dispatch
	* Author:   amity
	* Date:     Wed Sep 23 00:38:50 2026
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
#include <TAL/TAL.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static _PTAL_BACKEND ActiveBackend = NULL;

/* --- Prototypes ---*/

/* --- Functions ---*/

#define XAL_PREFIX TAL
#define XAL_BACKEND _PTAL_BACKEND
#define XAL_EMIT_DISPATCH
#include <XAL/xMCAL.H>
#include <TAL/TAL.xal>
#undef XAL_EMIT_DISPATCH
#undef XAL_METHOD
#undef XAL_METHOD_VOID
#undef XAL_PREFIX
#undef XAL_BACKEND