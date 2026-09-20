/*
	* Shadow/Include/IAL/IAL.H - Interrupt Abstraction Layer
	* Author:   amity
	* Date:     Sat Sep 19 00:06:01 2026
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
#ifndef __IAL_IAL_H__
#define __IAL_IAL_H__
/*
	* Old MacDonald had a farm, ✨IAL IAL H✨
*/

typedef struct IAL_BACKEND _IAL_BACKEND, *_PIAL_BACKEND;

#define XAL_PREFIX  IAL
#define XAL_BACKEND _PIAL_BACKEND

struct IAL_BACKEND {
	PCCHAR Name;
	#define XAL_EMIT_STRUCT
	#include <XAL/xMCAL.H>
	#include <IAL/IAL.xal>
	#undef XAL_EMIT_STRUCT
	#undef XAL_METHOD
	#undef XAL_METHOD_VOID
};

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

#define XAL_EMIT_PROTOTYPES
#include <XAL/xMCAL.H>
#include <IAL/IAL.xal>
#undef XAL_EMIT_PROTOTYPES
#undef XAL_METHOD
#undef XAL_METHOD_VOID
 
#undef XAL_PREFIX
#undef XAL_BACKEND

#endif /* __IAL_IAL_H__ */
