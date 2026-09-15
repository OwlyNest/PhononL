/*
	* Shadow/Include/GAL/GOP.H - GAL backed for UEFI GOP framebuffer
	* Author:   amity
	* Date:     Mon Sep 14 12:40:49 2026
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
#ifndef __GAL_GOP_H__
#define __GAL_GOP_H__
/* --- Includes ---*/
#include <GAL/GAL.H>
#include <info.h>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
/* 
 * Wraps whatever the bootloader already found via GOP and handedof in PhononBootInfo
 * Call once at boot, then GALSetBackend() the result
*/
_PGAL_BACKEND GAL_GOPBackend(IN PPhononBootInfo Info);
#endif /* __GAL_GOP_H__ */