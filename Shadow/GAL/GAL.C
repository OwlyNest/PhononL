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
#include <GAL/GOP/GOP.H>
#include <info.h>
#include <MM/MM.H>
#include <GFX/GFX.H>
#include <XAL/XScope.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
extern PhononBootInfo BootInfo;
static _GAL_BACKEND *ActiveBackend = NULL;

/* --- Prototypes ---*/

/* --- Functions ---*/
#define XAL_PREFIX GAL
#define XAL_BACKEND _PGAL_BACKEND
#define XAL_EMIT_DISPATCH
#include <XAL/xMCAL.H>
#include <GAL/GAL.xal>
#undef XAL_EMIT_DISPATCH
#undef XAL_METHOD
#undef XAL_METHOD_VOID
#undef XAL_PREFIX
#undef XAL_BACKEND


/*
	* This is safe. GCC picks the first. Defined, if GOP and some GPU from AMD are compiled in,
	* and GOP is at the top, then the init function that uses GOP does not override and the AMD GPU is initialized
*/
static SHSTATUS XScopeGALInit(VOID) {
#ifdef __GAL_GOP__
	SIZE_T FbBytes = (SIZE_T)BootInfo.framebuffer_pitch * BootInfo.framebuffer_height;
 
	/*
		* Write-combining, not uncached. The firmware hands the GOP surface
		* over as UC, where every pixel write is a separate bus
		* transaction -- which is why console output on real hardware
		* crawls. WC lets the CPU batch them, and needs the PAT slot that
		* MmInitPaging programmed.
	*/

	VIRT_ADDR_T FbVirt = MmMapIoSpace(MmGetKernelAddressSpace(), BootInfo.framebuffer_base, FbBytes, (_MM_PROTECTION)(MM_PROT_READ | MM_PROT_WRITE | MM_PROT_WRITECOMBINE));
 
	if (FbVirt == MM_VIRT_INVALID) {
		/* No framebuffer means no output of any kind from here on. */
		for (;;) {
			__asm__ __volatile__("cli\n\thlt");
		}
	}
 
	GAL_GOPSetVirtualBase(FbVirt);
	FbUpdateHw();

	GALSetBackend(GAL_GOPBackend(&BootInfo));
	


#else
#endif /* __GAL_XXX__ */
	return STATUS_SUCCESS;
}


XSCOPENODE(GAL, XScopeGALInit, "MM_VMM");