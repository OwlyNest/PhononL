/*
	* Shadow/IAL/IAL.C - Interrupt Abstraction Layer: dispatch
	* Author:   amity
	* Date:     Sat Sep 19 00:06:20 2026
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
#include <XAL/XScope.H>
#include <IAL/IAL.H>
#include <IAL/PIC/PIC.H>
#include <Lib/Lib.H>

/* --- Typedefs - Structs - Enums ---*/
static _PIAL_BACKEND ActiveBackend = NULL;

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/

#define XAL_PREFIX IAL
#define XAL_BACKEND _PIAL_BACKEND
#define XAL_EMIT_DISPATCH
#include <XAL/xMCAL.H>
#include <IAL/IAL.xal>
#undef XAL_EMIT_DISPATCH
#undef XAL_METHOD
#undef XAL_METHOD_VOID
#undef XAL_PREFIX
#undef XAL_BACKEND

#ifdef __IAL_PIC__
static SHSTATUS XScopeIALInit(VOID) {
	IALSetBackend(IALPicBackend());
	if (IALInit() != 0) {
        printk("[!] IAL backend '%s' failed to initialize\r\n", IALBackendName());
        for (;;) { __asm__ __volatile__("cli\n\thlt"); }
    }
    printk("[Ial] Backend: %s\r\n", IALBackendName());

    __asm__ __volatile__("sti");
	return STATUS_SUCCESS;
}
#else 
static SHSTATUS XScopeIALInit(VOID) {
	return STATUS_SUCCESS; /* No backend compiled */
}
#endif

XSCOPENODE(IAL, XScopeIALInit, "SERIAL", "X64_IDT");