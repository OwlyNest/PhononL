/*
	* Shadow/MM/PAT.C - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 12:27:39 2026
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
#include <Int/Int.H>
#include <MM/MM.H>
#include <Lib/Lib.H>
#include <XAL/XScope.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static BOOLEAN PatPresent;

/* --- Prototypes ---*/

/* --- Functions ---*/
static SHSTATUS PatInit(VOID) {
	PatPresent = FALSE;
	if (!CpuidGet()->Features.Pat) {
		/* The intel pentium 3 was a 32 bit CPU, how did you even get this far? */
		printk("Page Attribute Table not available\r\n");
		return STATUS_SUCCESS; /* No PAT, survivable */
	}

	PagingWriteMsr(MSR_IA32_PAT, PAT_CONFIGURATION);
	PatPresent = TRUE;
	return STATUS_SUCCESS;
}

BOOLEAN PatAvailable(VOID) {
	return PatPresent;
}


XSCOPENODE(MM_PAT, PatInit, "MM_PMM");