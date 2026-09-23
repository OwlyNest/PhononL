/*
	* Shadow/MM/VMM.C - [Enter description]
	* Author:   amity
	* Date:     Wed Sep 16 15:09:49 2026
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

/*
	* One unmapped page between consecutive allocations. An overrun walks
	* into a fault with a readable address instead of quietly scribbling on
	* whatever was mapped next.
*/

#define VMM_GUARD_PAGES 1

/* --- Includes ---*/
#include <MM/MM.H>
#include <XAL/XScope.H>
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
static VIRT_ADDR_T VmmBumpMmio(IN _PMM_ADDRESS_SPACE Space, IN SIZE_T Pages);
static VIRT_ADDR_T VmmBumpHeap(IN _PMM_ADDRESS_SPACE Space, IN SIZE_T Pages);

/* --- Functions ---*/

/* ==========================================================================
 * Window cursors
 *
 * Bump allocation, and deliberately so for now: neither window ever
 * reuses an address. 16 TiB each means that is fine for a very long
 * time, and a real free-list here would be complexity with nothing yet
 * exercising it. The day something maps and unmaps BARs in a loop, this
 * is the function that grows a free list -- not its callers.
 * ======================================================================= */
static VIRT_ADDR_T VmmBumpMmio(
	IN _PMM_ADDRESS_SPACE Space,
	IN SIZE_T Pages
) {
	VIRT_ADDR_T Virt = Space->MmioNext;
	SIZE_T      Span = (Pages + VMM_GUARD_PAGES) * PAGE_SIZE;
 
	if ((Virt + Span) >= (MM_MMIO_BASE + MM_MMIO_SIZE)) {
		return MM_VIRT_INVALID;
	}
 
	Space->MmioNext = Virt + Span;
	return Virt;
}
 
static VIRT_ADDR_T VmmBumpHeap(
	IN _PMM_ADDRESS_SPACE Space,
	IN SIZE_T Pages
) {
	VIRT_ADDR_T Virt = Space->HeapNext;
	SIZE_T      Span = (Pages + VMM_GUARD_PAGES) * PAGE_SIZE;
 
	if ((Virt + Span) >= (MM_HEAP_BASE + MM_HEAP_SIZE)) {
		return MM_VIRT_INVALID;
	}
 
	Space->HeapNext = Virt + Span;
	return Virt;
}
 
/* ==========================================================================
 * Ranges over existing physical memory
 * ======================================================================= */
SHSTATUS MmMapRange(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN PHYS_ADDR_T Phys,
	IN SIZE_T Bytes,
	IN _MM_PROTECTION Prot
) {
	SIZE_T Pages = PAGES_FOR(Bytes);
 
	for (SIZE_T i = 0; i < Pages; i++) {
		VIRT_ADDR_T V = Virt + (i * PAGE_SIZE);
		PHYS_ADDR_T P = Phys + (i * PAGE_SIZE);
 
		if (MmMapPage(Space, V, P, Prot) != STATUS_SUCCESS) {
			/* 
				* Roll back whatever landed, so a partial mapping never
				* outlives the failed call.
			*/

			MmUnmapRange(Space, Virt, i * PAGE_SIZE);
			return (SHSTATUS)-1;
		}
	}
 
	return STATUS_SUCCESS;
}
 
VOID MmUnmapRange(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN SIZE_T Bytes
) {
	SIZE_T Pages = PAGES_FOR(Bytes);
 
	for (SIZE_T i = 0; i < Pages; i++) {
		MmUnmapPage(Space, Virt + (i * PAGE_SIZE));
	}
}
 
/* ==========================================================================
 * Device memory
 * ======================================================================= */
VIRT_ADDR_T MmMapIoSpace(
	IN _PMM_ADDRESS_SPACE Space,
	IN PHYS_ADDR_T Phys,
	IN SIZE_T Bytes,
	IN _MM_PROTECTION Prot
) {
	/*
		* Device physical addresses are rarely page-aligned; preserve the
		* offset so the returned pointer lands on the right byte.
	*/
	
	PHYS_ADDR_T Aligned = PAGE_ALIGN_DOWN(Phys);
	SIZE_T      Offset  = (SIZE_T)(Phys - Aligned);
	SIZE_T      Pages   = PAGES_FOR(Bytes + Offset);
 
	/*
		* Device memory is never write-back cacheable. Uncached unless the
		* caller explicitly asked for write-combining, which is the right
		* choice for a framebuffer and the wrong one for control registers.
	*/

	if (!(Prot & (_MM_PROTECTION)MM_PROT_WRITECOMBINE)) {
		Prot |= (_MM_PROTECTION)MM_PROT_NOCACHE;
	}
 
	VIRT_ADDR_T Virt = VmmBumpMmio(Space, Pages);
	if (Virt == MM_VIRT_INVALID) {
		return MM_VIRT_INVALID;
	}
 
	if (MmMapRange(Space, Virt, Aligned, Pages * PAGE_SIZE, (_MM_PROTECTION)(Prot | MM_PROT_GLOBAL)) != STATUS_SUCCESS) {
		return MM_VIRT_INVALID;
	}
 
	return Virt + Offset;
}
 
VOID MmUnmapIoSpace(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN SIZE_T Bytes
) {
	VIRT_ADDR_T Aligned = PAGE_ALIGN_DOWN(Virt);
	SIZE_T      Offset  = (SIZE_T)(Virt - Aligned);
 
	MmUnmapRange(Space, Aligned, Bytes + Offset);
}
 
/* ==========================================================================
 * Backed virtual memory
 * ======================================================================= */
VIRT_ADDR_T MmAllocateVirtual(
	IN _PMM_ADDRESS_SPACE Space,
	IN SIZE_T Bytes,
	IN _MM_PROTECTION Prot
) {
	SIZE_T Pages = PAGES_FOR(Bytes);
 
	if (Pages == 0) {
		return MM_VIRT_INVALID;
	}
 
	VIRT_ADDR_T Virt = VmmBumpHeap(Space, Pages);
	if (Virt == MM_VIRT_INVALID) {
		return MM_VIRT_INVALID;
	}
 
	/*
		* Frames are allocated one at a time and mapped individually:
		* nothing here needs physical contiguity, and demanding it would
		* fail on a fragmented machine for no benefit. Anything that DOES
		* need contiguity (DMA buffers, later) wants MmAllocatePages and
		* MmMapRange instead.
	*/

	for (SIZE_T i = 0; i < Pages; i++) {
		PHYS_ADDR_T Phys = MmAllocatePage();
 
		if (Phys == PHYS_ADDR_INVALID) {
			/* Unwind: unmap and release everything already taken. */
			for (SIZE_T j = 0; j < i; j++) {
				VIRT_ADDR_T V = Virt + (j * PAGE_SIZE);
				PHYS_ADDR_T P = MmGetPhysicalAddress(Space, V);
 
				MmUnmapPage(Space, V);
 
				if (P != PHYS_ADDR_INVALID) {
					MmFreePage(P);
				}
			}

			return MM_VIRT_INVALID;
		}
 
		if (MmMapPage(Space, Virt + (i * PAGE_SIZE), Phys, (_MM_PROTECTION)(Prot | MM_PROT_GLOBAL)) != STATUS_SUCCESS) {
			MmFreePage(Phys);
			return MM_VIRT_INVALID;
		}
	}
 
	return Virt;
}
 
VOID MmFreeVirtual(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN SIZE_T Bytes
) {
	SIZE_T Pages = PAGES_FOR(Bytes);
 
	for (SIZE_T i = 0; i < Pages; i++) {
		VIRT_ADDR_T V = Virt + (i * PAGE_SIZE);
		PHYS_ADDR_T P = MmGetPhysicalAddress(Space, V);
 
		MmUnmapPage(Space, V);
 
		if (P != PHYS_ADDR_INVALID) {
			MmFreePage(P);
		}
	}
}

SHSTATUS MmVmmXScopeInit(VOID) {
	return STATUS_SUCCESS;	
}
 
XSCOPENODE(MM_VMM, MmVmmXScopeInit, "MM_Paging");