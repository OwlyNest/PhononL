/*
	* Shadow/Include/XAL/xMCAL.H - shared x-Multi-Complexity-Abstraction-Layer struct/dispatch generation
	* Author:   amity
	* Date:     Sun Sep 20 12:08:27 2026
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
	* This is the actual guarantee that every xMCAL looks the same: there
	* is exactly one definition of what a backend struct and its dispatch
	* functions look like, and every xMCAL (GAL, IAL, whatever comes next)
	* is generated from it -- not hand-written to independently match it.
	*
	* Deliberately NOT include-guarded: a subsystem includes this file up
	* to three times in one translation unit (struct, prototypes,
	* dispatch), redefining XAL_EMIT_* and re-including its own .xal file
	* between each. The caller is responsible for #undef'ing XAL_METHOD,
	* XAL_METHOD_VOID and whichever XAL_EMIT_* it set, after each pass --
	* this file only ever defines them, on the assumption the previous
	* pass already cleaned up after itself.
	*
	* What a subsystem provides before including this file:
	*   XAL_PREFIX   e.g. Gal       -- dispatch fns become GalInit, ...
	*   XAL_BACKEND  e.g. _PGAL_BACKEND
	* and exactly one of:
	*   XAL_EMIT_STRUCT       (from the .H, inside the backend struct body)
	*   XAL_EMIT_PROTOTYPES   (from the .H, after the struct)
	*   XAL_EMIT_DISPATCH     (from the .C)
	*
	* A subsystem's .C file MUST name its active-backend pointer exactly
	* ActiveBackend -- the dispatch template refers to it directly, not
	* through a parameter, to keep the generated functions' signatures
	* clean.
*/

/* --- Macros ---*/

#include <XAL/xALCommon.H>
#define XAL_FN(Name)      XAL_CONCAT(XAL_PREFIX, Name)

/* 
	* Metadata lines (Class, Depends, ...) are read by the build-time
	* generator as plain text, via a script that never runs a real
	* preprocessor over this file. To the actual compiler they are
	* unconditionally nothing, in every emit pass.
*/
#define XAL_META(Key, Value)

#if defined(XAL_EMIT_STRUCT)

    #define XAL_METHOD(Ret, Name, StructArgs, PublicArgs, CallArgs, Fail) \
        Ret (*Name) StructArgs;
    #define XAL_METHOD_VOID(Name, StructArgs, PublicArgs, CallArgs) \
        VOID (*Name) StructArgs;

#elif defined(XAL_EMIT_PROTOTYPES)

    VOID   XAL_FN(SetBackend)(IN XAL_BACKEND Backend);
    PCCHAR XAL_FN(BackendName)(VOID);

    #define XAL_METHOD(Ret, Name, StructArgs, PublicArgs, CallArgs, Fail) \
        Ret XAL_FN(Name) PublicArgs;
    #define XAL_METHOD_VOID(Name, StructArgs, PublicArgs, CallArgs) \
        VOID XAL_FN(Name) PublicArgs;

#elif defined(XAL_EMIT_DISPATCH)

    VOID XAL_FN(SetBackend)(IN XAL_BACKEND Backend) {
        ActiveBackend = Backend;
    }

    PCCHAR XAL_FN(BackendName)(VOID) {
        return ActiveBackend ? ActiveBackend->Name : "None";
    }

    #define XAL_METHOD(Ret, Name, StructArgs, PublicArgs, CallArgs, Fail) \
        Ret XAL_FN(Name) PublicArgs {                      \
            if (!ActiveBackend) {                          \
                return (Fail);                             \
            }                                              \
            return ActiveBackend->Name CallArgs;           \
        }
    #define XAL_METHOD_VOID(Name, StructArgs, PublicArgs, CallArgs) \
        VOID XAL_FN(Name) PublicArgs {                     \
            if (!ActiveBackend) {                          \
                return;                                    \
            }                                              \
            ActiveBackend->Name CallArgs;                  \
        }

#else
    #error "XMcal.H requires XAL_EMIT_STRUCT, XAL_EMIT_PROTOTYPES, or XAL_EMIT_DISPATCH"
#endif

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/