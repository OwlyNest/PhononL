/*
	* Shadow/Include/MM/PMM.H - [Enter description]
	* Author:   amity
	* Date:     Wed Sep 16 09:49:07 2026
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
#ifndef __MM_PMM_H__
#define __MM_PMM_H__

/* --- Includes ---*/
#include <MM/MM.H>
#include <info.h>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* ==========================================================================
 * Initialization
 *
 * Runs BEFORE paging is ours — still on firmware's identity map, so
 * physical == virtual in here. Nothing in this file may use
 * MmPhysToVirt().
 * ======================================================================= */

SHSTATUS MmInitPhysical(IN PPhononBootInfo Info);
 
/* ==========================================================================
 * Allocation
 *
 * All of these deal in PHYSICAL addresses. Turning one into something
 * dereferenceable is the caller's job (MmPhysToVirt, once paging is up).
 * Returns PHYS_ADDR_INVALID on failure.
 * ======================================================================= */

PHYS_ADDR_T MmAllocatePage(VOID);
PHYS_ADDR_T MmAllocatePages(IN SIZE_T Count);   /* physically contiguous */
 
VOID MmFreePage(IN PHYS_ADDR_T Phys);
VOID MmFreePages(IN PHYS_ADDR_T Phys, IN SIZE_T Count);
 
/* ==========================================================================
 * Reservation
 *
 * Mark a physical range as permanently unavailable — a framebuffer the
 * firmware handed us, an MMIO BAR, anything the allocator must never
 * hand out. Safe to call on ranges that are already reserved.
 * ======================================================================= */

VOID MmReservePhysicalRange(IN PHYS_ADDR_T Base, IN SIZE_T Bytes);
 
/* ==========================================================================
 * Introspection
 * ======================================================================= */

VOID MmGetPhysicalStats(OUT _PMM_STATS Stats);

/*
	* Called by MmInitPaging immediately after the CR3 switch. The bitmap
	* pointer captured during MmInitPhysical was an identity-mapped physical
	* address; this re-derives it through the direct map. Nothing may call
	* into the PMM between the switch and this.
*/

VOID MmPmmRebaseToDirectMap(VOID);

/*
	* Reclaims EfiBootServicesCode/Data. Call ONLY after MmInitPaging() has
	* switched CR3 and the framebuffer has been remapped -- before that,
	* this memory may still hold code or a mapping the CPU is currently
	* relying on, and this function itself calls printk. This is the same
	* thing Windows' and Linux's own loaders do; the only difference is
	* exactly which "safe point" triggers it, since neither of them needs a
	* higher-half switch of its own first.
*/

VOID MmReclaimBootServices(IN PPhononBootInfo Info);

#endif /* __MM_PMM_H__ */