/*
 * Boot/Handoff.c - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:46:59 2026
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

/* --- Includes ---*/
#include <Uefi.h>

#include <Handoff.h>
/* --- Typedefs - Structs - Enums ---*/

/* Deliberately NOT EFIAPI: EFIAPI forces the Microsoft x64 calling
 * convention (arguments in RCX/RDX/...), which is what UEFI protocol
 * calls use under the hood. Shadow's _start is a plain SysV AMD64
 * function — first argument in RDI — which is GCC's default calling
 * convention on this target when no attribute overrides it. Tagging this
 * EFIAPI would silently mismatch the ABI Shadow's entry point expects.
*/
typedef VOID (*PHONON_KERNEL_ENTRY)(
    PPhononBootInfo Info
);

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/
VOID HandoffJump(
    IN VOID *KernelEntry,
    IN OUT PPhononBootInfo Info
) {
    PHONON_KERNEL_ENTRY Entry = (PHONON_KERNEL_ENTRY)(UINTN)KernelEntry;

    Entry(Info);

    /* Should never reach here — Shadow's entry point isn't supposed to
     * return. If it somehow does, there's nothing left to fall back to
     * (boot services are gone), so just stop the CPU cleanly.
    */
    for (;;) {
        __asm__ __volatile__("cli\n\thlt");
    }
}