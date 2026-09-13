/*
 * Boot/Memory.h - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:46:26 2026
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
#ifndef __MEMORY_H__
#define __MEMORY_H__
/* --- Includes ---*/   
#include <Uefi.h>

#include <info.h>
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
EFI_STATUS MemoryExitBootServices(
    IN EFI_HANDLE ImageHandle,
    IN OUT PPhononBootInfo Info
);

#endif /* __MEMORY_H__ */