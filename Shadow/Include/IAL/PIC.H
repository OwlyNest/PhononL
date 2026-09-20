/*
	* Shadow/Include/IAL/PIC.H - IAL backend for the 8259 PIC
	* Author:   amity
	* Date:     Sat Sep 19 00:06:05 2026
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
#ifndef __IAL_PIC_H__
#define __IAL_PIC_H__

/* --- Includes ---*/
#include <IAL/IAL.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
_PIAL_BACKEND IALPicBackend(VOID);

#endif /* __IAL_PIC_H__ */