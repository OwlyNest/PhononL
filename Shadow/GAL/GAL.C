/*
	* Shadow/GAL/GAL.C - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 14 12:40:04 2026
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
#include <GAL/GAL.H>
#include <Lib/PrintK.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static _GAL_BACKEND *ActiveBackend = NULL;

/* --- Prototypes ---*/

/* --- Functions ---*/
/* ==========================================================================
 * Registration
 * ======================================================================= */
VOID GALSetBackend(
	IN _PGAL_BACKEND Backend
) {
	ActiveBackend = Backend;
}

PCCHAR GALBackendName(VOID) {
	return ActiveBackend ? ActiveBackend->Name : "None";
}

/* ==========================================================================
 * Dispatch
 * ======================================================================= */
INT GALInit(void) {
  	if (!ActiveBackend) {
    	return -1;
	}
	return ActiveBackend->Init(ActiveBackend);
}

VOID GALGetMode(
	OUT _PGAL_MODE OutMode
) {
	if (!ActiveBackend) {
		return;
	}
	ActiveBackend->GetMode(ActiveBackend, OutMode);
}

PVOID GALGetFramebuffer(VOID) {
	if (!ActiveBackend) {
		return NULL;
	}

	return ActiveBackend->GetFramebuffer(ActiveBackend);
}

VOID GALPresent(const _PGFX_SURFACE Back) {
	if (!ActiveBackend) {
		return;
	}

	ActiveBackend->Present(ActiveBackend, Back);
}