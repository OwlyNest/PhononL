/*
	* Shadow/Include/MM/VMM.H - [Enter description]
	* Author:   amity
	* Date:     Wed Sep 16 15:09:44 2026
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
#ifndef __MM_VMM_H__
#define __MM_VMM_H__

#define MM_VIRT_INVALID ((VIRT_ADDR_T)0)

/* --- Includes ---*/
#include <MM/MM.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* ==========================================================================
 * Ranges over existing physical memory
 *
 * Neither of these allocates frames: the caller already has the physical
 * memory and is deciding where it appears.
 * ======================================================================= */

SHSTATUS MmMapRange(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN PHYS_ADDR_T Phys,
	IN SIZE_T Bytes,
	IN _MM_PROTECTION Prot
);
 
VOID MmUnmapRange(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN SIZE_T Bytes
);
 
/* ==========================================================================
 * Device memory
 *
 * Picks an address in the MMIO window and maps Bytes of device physical
 * memory there. This is how the framebuffer survives MmInitPaging
 * dropping the identity map, and how PCIe BARs will be reached later.
 *
 * Returns MM_VIRT_INVALID on failure. Prot picks up MM_PROT_NOCACHE
 * automatically unless MM_PROT_WRITECOMBINE is asked for.
 * ======================================================================= */

VIRT_ADDR_T MmMapIoSpace(
	IN _PMM_ADDRESS_SPACE Space,
	IN PHYS_ADDR_T Phys,
	IN SIZE_T Bytes,
	IN _MM_PROTECTION Prot
);
 
VOID MmUnmapIoSpace(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN SIZE_T Bytes
);
 
/* ==========================================================================
 * Backed virtual memory
 *
 * Allocates both the address range AND the frames behind it. This is what
 * the heap grows through -- the heap suballocates inside what this hands
 * back and never talks to the PMM or the page tables directly.
 * ======================================================================= */

VIRT_ADDR_T MmAllocateVirtual(
	IN _PMM_ADDRESS_SPACE Space,
	IN SIZE_T Bytes,
	IN _MM_PROTECTION Prot
);
 
VOID MmFreeVirtual(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN SIZE_T Bytes
);


#endif /* __MM_VMM_H__ */