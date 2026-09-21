/*
	* Shadow/Include/XAL/xSCAL.H - shared xSCAL prototype generation
	* Author:   amity
	* Date:     Sun Sep 20 22:37:51 2026
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
/*
	* xSCAL has no backend indirection to generate, one implementation
	* (possibly split across several complementary files), called
	* directly, under its real name. What this formalizes is narrower and
	* purely a naming contract: a function is a subsystem's public surface
	* if and only if it's declared here. Everything else in the
	* subsystem's .C files must be static. CheckSurface.ps1 is what
	* actually enforces that half, since C itself has no way to.
	*
	* The half C DOES enforce for free: if a .xal-declared function's
	* real definition doesn't match the prototype this file generates,
	* the build fails right there. Manifest and implementation cannot
	* silently drift apart once this is wired in -- only "wired in" is
	* opt-in, unlike xMCAL where the struct forces it structurally.
	*
	* Usage, once per subsystem, in its one canonical public header:
	*   #define XAL_EMIT_PROTOTYPES
	*   #include <Xal/XScal.H>
	*   #include "MM.xal"
	*   #undef XAL_EMIT_PROTOTYPES
	*   #undef XAL_METHOD
	*
	* No XAL_PREFIX, no XAL_BACKEND. Names in a .xal file are already
	* final.
*/

/* --- Macros ---*/
#define XAL_META(Key, Value)
 
#if defined(XAL_EMIT_PROTOTYPES)
 
	#define XAL_METHOD(Ret, Name, Args) Ret Name Args;
 
#else
	#error "XScal.H requires XAL_EMIT_PROTOTYPES"
#endif

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
