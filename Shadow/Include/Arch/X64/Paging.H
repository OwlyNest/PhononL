/*
	* Shadow/Include/Arch/X64/Paging.H - [Enter description]
	* Author:   amity
	* Date:     Wed Sep 16 14:29:12 2026
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
#ifndef __ARCH_X64_PAGING_H__
#define __ARCH_X64_PAGING_H__

/* --- Includes ---*/
#include <MM/MM.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* ==========================================================================
 * Initialization
 *
 * Replaces the boot stub's crude static tables with real ones: a direct
 * map sized to actual RAM, kernel sections mapped at their true
 * permissions, and no identity mapping at all.
 *
 * Requires MmInitPhysical() first (it allocates page-table frames), and
 * invalidates every raw physical pointer taken before it -- see the
 * PmmRebase note in PMM.H.
 * ======================================================================= */

SHSTATUS MmInitPaging(VOID);
 
_PMM_ADDRESS_SPACE MmGetKernelAddressSpace(VOID);
 
/* ==========================================================================
 * Mapping
 *
 * Virt and Phys must both be page-aligned. MmMapHugePage maps 1 GiB and
 * wants 1 GiB alignment on both.
 * ======================================================================= */

SHSTATUS MmMapPage(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN PHYS_ADDR_T Phys,
	IN _MM_PROTECTION Prot
);
 
SHSTATUS MmMapHugePage(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN PHYS_ADDR_T Phys,
	IN _MM_PROTECTION Prot
);
 
VOID MmUnmapPage(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt
);
 
/*
	* PHYS_ADDR_INVALID if nothing is mapped there.
*/

PHYS_ADDR_T MmGetPhysicalAddress(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt
);
 
/* ==========================================================================
 * TLB / CR3
 * ======================================================================= */
VOID MmSwitchAddressSpace(IN _PMM_ADDRESS_SPACE Space);
VOID MmInvalidatePage(IN VIRT_ADDR_T Virt);
VOID MmFlushTlb(VOID);


#endif /* __ARCH_X64_PAGING_H__ */