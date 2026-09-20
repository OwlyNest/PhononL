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
#include <GFX/FB.H>

/* --- Typedefs - Structs - Enums ---*/
/*
 * Generic description of whatever mode the active backend is currently
 * driving. Anything above the GAL that needs to know about pixel layout
 * reads this, nothing should ever need to know which concrete backend
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

#define XAL_PREFIX  GAL
#define XAL_BACKEND _PGAL_BACKEND
 
/*
 * Every backend implements this. Nothing outside GAL.C ever calls
 * through these function pointers directly; see the dispatch functions
 * below. Adding a new GPU means writing a new _GAL_BACKEND here in
 * GAL/, not editing FB.C or anything downstream of it. Struct shape
 * comes from GAL.xal via Xal/XMcal.H — not hand-written here, so it
 * cannot drift from what GAL.C's dispatch actually forwards to.
*/

struct GAL_BACKEND {
	PCCHAR Name;
	#define XAL_EMIT_STRUCT
	#include <XAL/xMCAL.H>
	#include <GAL/GAL.xal>
	#undef XAL_EMIT_STRUCT
	#undef XAL_METHOD
	#undef XAL_METHOD_VOID
};
 
/* --- Globals ---*/
 
/* --- Prototypes ---*/
/* ==========================================================================
 * Registration + Dispatch — both generated from GAL.xal. Callers never
 * touch a backend's function pointers directly, and never null-check
 * "is a backend set?" themselves; that shape lives in Xal/XMcal.H once,
 * not rewritten per subsystem.
 * ======================================================================= */

 #define XAL_EMIT_PROTOTYPES
#include <XAL/xMCAL.H>
#include <GAL/GAL.xal>
#undef XAL_EMIT_PROTOTYPES
#undef XAL_METHOD
#undef XAL_METHOD_VOID
 
#undef XAL_PREFIX
#undef XAL_BACKEND

#endif /* __GAL_GAL_H__ */