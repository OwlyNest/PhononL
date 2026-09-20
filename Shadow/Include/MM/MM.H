/*
	* Shadow/Include/MM/MM.H - Memory manager: layout, page macros, shared types
	* Author:   amity
	* Date:     Wed Sep 16 09:48:59 2026
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
	* Empty parameter list:          Use (VOID) instead of ()
	* Statements and declarations:   Max one per line
*/

/* --- Macros ---*/
#ifndef __MM_MM_H__
#define __MM_MM_H__

#define PAGE_SIZE          4096
#define PAGE_SHIFT         12
#define PAGE_MASK          (PAGE_SIZE - 1)

#define PAGE_ALIGN_DOWN(x) ((x) & ~((UINT_PTR)PAGE_MASK))
#define PAGE_ALIGN_UP(x)   PAGE_ALIGN_DOWN((x) + PAGE_MASK)
#define PAGES_FOR(Bytes)   (((Bytes) + PAGE_MASK) >> PAGE_SHIFT)

#define PFN_OF(Phys)       ((PFN_T)((Phys) >> PAGE_SHIFT))
#define PHYS_OF(Pfn)       ((PHYS_ADDR_T)(Pfn) << PAGE_SHIFT)

#define HUGE_1G_SIZE       0x40000000ULL
#define HUGE_2M_SIZE       0x00200000ULL

/*
	* Address space layout:
	* Canonical higher half only. Nothing below 0xFFFF800000000000 belongs to
	* the kernel: that space is reserved for user address spaces, once those
	* exist.
*/

/*
	* Every physical page is permanently visible here at
	* MM_DIRECT_MAP_BASE + PhysAddr. This is what replaces old Phonon's
	* PHYS_TO_VIRT / vmm_map_physical window dance: no mapping call, no
	* window to collide with, just arithmetic.
	*
	* NOT VALID until the boot stub has loaded its PML4. Anything running
	* before that is on firmware's identity map and must use physical
	* addresses directly. MmInitPhysical is the only such caller.
*/

#define MM_DIRECT_MAP_BASE  0xFFFF800000000000ULL
#define MM_DIRECT_MAP_SIZE  0x0000400000000000ULL  /* 64 TiB */

#define MM_MMIO_BASE        0xFFFFA00000000000ULL
#define MM_MMIO_SIZE        0x0000100000000000ULL

#define MM_HEAP_BASE        0xFFFFC00000000000ULL
#define MM_HEAP_SIZE        0x0000100000000000ULL

/*
	* Matches -mcmodel=kernel's assumption that all symbols live in the top
	* 2 GiB. Link.ld's KERNEL_VMA and this constant have to agree; the
	* C_ASSERT at the bottom of this file enforces it against the linker.
*/
#define MM_KERNEL_BASE      0xFFFFFFFF80000000ULL

/* --- Includes ---*/
#include <Internal/Types.H>

/* --- Typedefs - Structs - Enums ---*/

/*
	* Page protection / mapping flags. Deliberately NOT raw x86_64 PTE bit
	* positions: those live in the arch layer and get translated there. A
	* caller asking for a writable page should not have to know which bit
	* that is on this particular CPU.
*/
typedef enum MM_PROTECTION {
	MM_PROT_NONE    = 0x0000,
	MM_PROT_READ    = 0x0001,
	MM_PROT_WRITE   = 0x0002,
	MM_PROT_EXECUTE = 0x0004,
	MM_PROT_USER    = 0x0008,          /* accessible from ring 3 */
	MM_PROT_GLOBAL  = 0x0010,          /* survives CR3 reload */
	MM_PROT_NOCACHE = 0x0020,          /* MMIO, control registers */
	MM_PROT_WRITECOMBINE = 0x0040,     /* framebuffers, big win on real HW */
} _MM_PROTECTION, *_PMM_PROTECTION;

/*
	* An address space. TopLevelTable is deliberately not called Pml4: on
	* another architecture it is a TTBR or an SATP, and nothing above the
	* arch layer should have an opinion about which.
	*
	* MmioNext / HeapNext are bump cursors into the two kernel VA windows.
	* One allocator per window, living here, is what stops the heap and the
	* MMIO mapper from ever handing out the same address.
*/
typedef struct MM_ADDRESS_SPACE {
	PHYS_ADDR_T TopLevelTable;
	VIRT_ADDR_T MmioNext;
	VIRT_ADDR_T HeapNext;
} _MM_ADDRESS_SPACE, *_PMM_ADDRESS_SPACE;

typedef struct MM_STATS {
	SIZE_T TotalPages;
	SIZE_T FreePages;
	SIZE_T UsedPages;
	SIZE_T ReservedPages;              /* firmware, ACPI, MMIO, never ours */
	PHYS_ADDR_T HighestPhysical;
} _MM_STATS, *_PMM_STATS;

/* --- Globals ---*/

/*
	* Emitted by Link.ld. Addresses, not values: always take &__TextStart
	* style by using them as arrays, never dereference them.
*/
extern UINT8 __KernelVirtStart[];
extern UINT8 __KernelVirtEnd[];
extern UINT8 __TextStart[];
extern UINT8 __TextEnd[];
extern UINT8 __RodataStart[];
extern UINT8 __RodataEnd[];
extern UINT8 __XScopeNodesStart[];
extern UINT8 __XScopeNodesEnd[];
extern UINT8 __DataStart[];
extern UINT8 __DataEnd[];
extern UINT8 __BssStart[];
extern UINT8 __BssEnd[];
extern UINT8 __KernelVmaMarker[];      /* == KERNEL_VMA from Link.ld */

/* --- Prototypes ---*/

/*
	* Valid only once the boot stub's PML4 is live. See the direct map note
	* above.
*/
FORCEINLINE PVOID MmPhysToVirt(IN PHYS_ADDR_T Phys) {
	return (PVOID)(MM_DIRECT_MAP_BASE + Phys);
}

FORCEINLINE PHYS_ADDR_T MmVirtToPhys(IN PVOID Virt) {
	return (PHYS_ADDR_T)((UINT_PTR)Virt - MM_DIRECT_MAP_BASE);
}

/*
	* For kernel-image addresses only (linker symbols, function pointers),
	* which are mapped at MM_KERNEL_BASE rather than through the direct map.
*/
FORCEINLINE PHYS_ADDR_T MmKernelVirtToPhys(IN PVOID Virt) {
	return (PHYS_ADDR_T)((UINT_PTR)Virt - MM_KERNEL_BASE);
}

#endif /* __MM_MM_H__ */
