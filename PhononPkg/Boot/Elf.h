/*
 * Boot/Elf.h - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:56:56 2026
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
#ifndef __ELF_H__
#define __ELF_H__
/* --- Includes ---*/
#include <Uefi.h>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
EFI_STATUS ElfLoadImage(
    IN VOID   *FileBuffer,
    IN UINTN   FileSize,
    OUT VOID **EntryPoint
);

#endif /* __ELF_H__ */