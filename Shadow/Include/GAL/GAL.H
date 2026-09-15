/*
	* Shadow/Include/GAL/GAL.H - Graphics Abstraction Layer
	* Author:   amity
	* Date:     Mon Sep 14 12:39:36 2026
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
#ifndef __GAL_GAL_H__
#define __GAL_GAL_H__

/* --- Includes ---*/
#include "Internal/Types.H"
#include <GFX/FB.H>

/* --- Typedefs - Structs - Enums ---*/
/*
 * Generic description of whatever mode the active backend is currently
 * driving. Anything above the GAL that needs to know about pixel layout
 * reads this — nothing should ever need to know which concrete backend
 * produced it.
*/
typedef struct GAL_MODE {
	UINT32 Width;
	UINT32 Height;
	UINT32 Pitch; /* bytes per scanline */
	UINT32 Bpp;
	UINT32 RedMask;
	UINT32 GreenMask;
	UINT32 BlueMask;
	UINT32 RedPos;
	UINT32 GreenPos;
	UINT32 BluePos;
} _GAL_MODE, *_PGAL_MODE;

typedef struct GAL_BACKEND _GAL_BACKEND, *_PGAL_BACKEND;

/*
 * Every backend implements this. Nothing outside GAL.C ever calls
 * through these function pointers directly; see the dispatch functions
 * below. Adding a new GPU means writing a new _GAL_BACKEND here in
 * GAL/, not editing FB.C or anything downstream of it.
*/
struct GAL_BACKEND {
	PCCHAR Name; /* For logging only */

	/*
	 * Returns 0 on success, nonzero if this backend isn't usable right now.
	 * Selection code (GALSelect(), once more than one backend exists)) tries
	 * backend in priority order untill one succeeds.
	*/

	INT (*Init)(IN _GAL_BACKEND *Self);
	VOID (*GetMode)(IN _GAL_BACKEND *Self, INOUT _GAL_MODE *OutMode);

	/* Linear framebuffer pointer, or NULL if this backend has no direct
   	 * memory-mapped access.
	*/
	VOID *(* GetFramebuffer)(IN _GAL_BACKEND *Self);

	/* Called after the generic backbuffer copy. Anything backend-specific
     * about actually presenting (a GPU FIFO kick, for example) belongs
     * here; a plain linear-framebuffer backend can leave this a no-op.
	*/
  	VOID (*Present)(IN _GAL_BACKEND *Self, IN const _PGFX_SURFACE Back);
};

/* --- Globals ---*/

/* --- Prototypes ---*/
/* ==========================================================================
 * Registration — the one place allowed to know which concrete backend
 * won. Everything else calls the dispatch functions below instead.
 * ======================================================================= */
VOID GALSetBackend(IN _PGAL_BACKEND Backend);
PCCHAR GALBackendName(VOID); /* "none" if nothing set yet */
 
/* ==========================================================================
 * Dispatch — callers never touch a gal_backend_t's function pointers
 * directly, and never need to null-check "is a backend set?" themselves.
 * ======================================================================= */
INT GALInit(VOID);
VOID  GALGetMode(INOUT _PGAL_MODE OutMode);
PVOID GALGetFramebuffer(VOID);
VOID  GALPresent(IN const _PGFX_SURFACE Back);


#endif /* __GAL_GAL_H__ */