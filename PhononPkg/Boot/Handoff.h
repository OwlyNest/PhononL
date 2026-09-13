/*
 * Boot/Handoff.h - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:47:06 2026
 * Copyright © 2026 OwlyNest
 */

/* --- Styling Instructions ---
 * Encoding:                      UTF-8, Unix line endings
 * Text font:                     Monospace
 * Line width:                    Max 80 characters
 * Indentation:                   Use 4 spaces
 * Brace style:                   Same line as control statement
 * Inline comments:               Column 40, wherever possible, else, whole
 * multiple of 20 Section headers:               Use 3 '-' characters before and
 * after Pointer notation:              Next to variable name, not type Binary
 * operations:             Space around operator Empty parameter list: Use
 * (void) instead of () Statements and declarations:   Max one per line
 */

/* --- Macros ---*/
#ifndef __HANDOFF_H__
#define __HANDOFF_H__
/* --- Includes ---*/
#include <Uefi.h>

#include <info.h>
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

// Never returns: jumps directly into KernelEntry and does not come back.
// If the kernel entry point ever does return, there is nothing sensible
// left to fall back to (boot services are long gone by this point).
VOID HandoffJump(
    IN VOID *KernelEntry,
    IN OUT PPhononBootInfo Info
);

#endif /* __HANDOFF_H__ */