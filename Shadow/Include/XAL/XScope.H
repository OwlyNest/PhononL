/*
	* Shadow/Include/XAL/XScope.H - dependency-ordered subsystem bring-up
	* Author:   amity
	* Date:     Sun Sep 20 13:37:17 2026
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
#ifndef __XAL_XSCOPE_H__
#define __XAL_XSCOPE_H__

/* --- Includes ---*/
#include <XAL/xALCommon.H>

/* --- Typedefs - Structs - Enums ---*/
typedef SHSTATUS (*XSCOPE_INIT_FN)(VOID);

typedef struct XSCOPE_NODE {
	PCCHAR Name;
	PCCHAR const *Depends;
	SIZE_T DependCount;
	XSCOPE_INIT_FN Init;
} _XScopeNode, *_PXScopeNode;
/* --- Globals ---*/

/* --- Prototypes ---*/

/*
	* Declares a node and places it in the .xscope_nodes linker section,
	* self-registering, so no central file has to list every subsystem in
	* the kernel (the exact coupling point this exists to avoid).
	* Dependencies are matched by Name at runtime, not by symbol reference:
	* a node never needs to know which translation unit its dependency
	* lives in.
	*
	*   XSCOPE_NODE(MM_Paging, MmPagingXScopeInit, "MM.PMM")
	*   XSCOPE_NODE(MM_PMM, MmPmmXScopeInit)   // no dependencies: fine
	*
	* Leans on GCC's empty-initializer-list extension for the no-deps
	* case, acceptable given this kernel is GCC/x86_64-elf-only
	* throughout already.
*/

#define XSCOPENODE(NodeName, InitFn, ...)                                  \
	static PCCHAR CONST XAL_CONCAT(NodeName, _XScopeDepends)[] =            \
		{ __VA_ARGS__ };                                                    \
	__attribute__((section(".xscope_nodes"), used))                        \
	static CONST _XScopeNode XAL_CONCAT(NodeName, _XScopeNode) = {         \
		.Name        = #NodeName,                                           \
		.Depends     = XAL_CONCAT(NodeName, _XScopeDepends),                \
		.DependCount = sizeof(XAL_CONCAT(NodeName, _XScopeDepends)) /       \
					   sizeof(PCCHAR),                                      \
		.Init        = InitFn,                                             \
	}

/*
	* Runs every registered node in dependency order: repeatedly scans for
	* any not-yet-run node whose dependencies are all satisfied, until
	* either everything has run or a full pass makes no progress (an
	* unresolved dependency name or a genuine cycle, reported by name,
	* not guessed at). Halts on the first Init that doesn't return
	* STATUS_SUCCESS: a subsystem that can't come up is not something to
	* silently continue past.
*/

VOID XScopeRun(VOID);


#endif /* __XAL_XSCOPE_H__ */