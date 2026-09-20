/*
	* Shadow/Arch/X64/Paging.C - x86_64 4-level page tables
	* Author:   amity
	* Date:     Wed Sep 16 14:28:58 2026
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
#define PTE_PRESENT     (1ULL << 0)
#define PTE_WRITABLE    (1ULL << 1)
#define PTE_USER        (1ULL << 2)
#define PTE_PWT         (1ULL << 3)
#define PTE_PCD         (1ULL << 4)
#define PTE_ACCESSED    (1ULL << 5)
#define PTE_DIRTY       (1ULL << 6)
#define PTE_PAGE_SIZE   (1ULL << 7)         /* huge page, on PDPT / PD */
#define PTE_PAT_4K      (1ULL << 7)         /* PAT selector, on PT only */
#define PTE_GLOBAL      (1ULL << 8)
#define PTE_NX          (1ULL << 63)
 
#define PTE_ADDR_MASK   0x000FFFFFFFFFF000ULL
 
#define PML4_INDEX(v)   (((v) >> 39) & 0x1FF)
#define PDPT_INDEX(v)   (((v) >> 30) & 0x1FF)
#define PD_INDEX(v)     (((v) >> 21) & 0x1FF)
#define PT_INDEX(v)     (((v) >> 12) & 0x1FF)
 
#define MSR_IA32_PAT    0x277

/*
	* PAT slots, low byte first:
	*   0 WB   1 WT   2 UC-  3 UC   4 WC   5 WT   6 UC-  7 UC
	*
	* Only slot 4 differs from the power-on default, and it is the whole
	* point: write-combining is unavailable out of reset, and an
	* uncached framebuffer is why drawing to the GOP surface on real
	* hardware crawls. Slot 4 is selected by PAT=1, PCD=0, PWT=0.
*/

#define PAT_CONFIGURATION 0x0007040100070406ULL


/* --- Includes ---*/
#include <Lib/String.H>
#include <MM/MM.H>
#include <Arch/X64/Paging.H>
#include <MM/PMM.H>
#include <XAL/XScope.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static _MM_ADDRESS_SPACE KernelSpace;

/* --- Prototypes ---*/
static UINT64      PagingProtToFlags(IN _MM_PROTECTION Prot, IN BOOLEAN Leaf4K);
static PHYS_ADDR_T PagingDescend(IN PHYS_ADDR_T TablePhys, IN SIZE_T Index, IN BOOLEAN Create);
static VOID        PagingWriteMsr(IN UINT32 Msr, IN UINT64 Value);
static VOID        PagingMapKernelSection(IN PVOID Start, IN PVOID End, IN _MM_PROTECTION Prot, IN PCCHAR Label);
/* --- Functions ---*/

/* ==========================================================================
 * CPU primitives
 * ======================================================================= */
VOID MmInvalidatePage(
	IN VIRT_ADDR_T Virt
) {
	__asm__ __volatile__ ("invlpg (%0)" :: "r"(Virt) : "memory");
}

VOID MmFlushTlb(VOID) {
	UINT64 Cr3;
	__asm__ __volatile__("movq %%cr3, %0" : "=r"(Cr3)); /* GAS syntax, mov content of cr3 into Cr3 variable, MOVq so, 64-bit 🦄*/
	__asm__ __volatile__("movq %0, %%cr3" :: "r"(Cr3) : "memory");
}

VOID MmSwitchAddressSpace(IN _PMM_ADDRESS_SPACE Space) {
	__asm__ __volatile__("movq %0, %%cr3" :: "r"((UINT64)Space->TopLevelTable) : "memory");
}
 
static VOID PagingWriteMsr(
	IN UINT32 Msr,
	IN UINT64 Value
) {
	__asm__ __volatile__("wrmsr" :: "c"(Msr), "a"((UINT32)(Value & 0xFFFFFFFF)), "d"((UINT32)(Value >> 32)));
}

/* ==========================================================================
 * Protection translation
 *
 * The one place _MM_PROTECTION turns into x86_64 bit positions. Callers
 * never see a PTE bit; this is what lets the flags stay meaningful on a
 * machine whose page tables look nothing like these.
 * ======================================================================= */

static UINT64 PagingProtToFlags(
	IN _MM_PROTECTION Prot,
	IN BOOLEAN Leaf4K
) {
	UINT64 Flags = PTE_PRESENT;

	if (Prot & MM_PROT_WRITE) {
		Flags |= PTE_WRITABLE;
	}

	if (Prot & MM_PROT_USER) {
		Flags |= PTE_USER;
	}

	if (Prot & MM_PROT_GLOBAL) {
		Flags |= PTE_GLOBAL;
	}

		/* x86_64 has no "no-read": absence of EXECUTE is what we can express. */
	if (!(Prot & MM_PROT_EXECUTE)) {
		Flags |= PTE_NX;
	}

		if (Prot & MM_PROT_NOCACHE) {
		Flags |= PTE_PCD;
	}
 
		if (Prot & MM_PROT_WRITECOMBINE) {
		/*
			* PAT slot 4. The selector bit sits at bit 7 for 4 KiB leaves and
			* bit 12 for huge leaves -- same idea, different position, which
			* is exactly the kind of detail worth keeping in one function.
		*/

		if (Leaf4K) {
			Flags |= PTE_PAT_4K;
		} else {
			Flags |= (1ULL << 12);
		}
	}
 
	return Flags;
}

/* ==========================================================================
 * Table walk
 *
 * Returns the physical address of the next level down, allocating and
 * zeroing it when Create is set. PHYS_ADDR_INVALID on failure or if the
 * entry is a huge leaf (nothing to descend into).
 * ======================================================================= */

static PHYS_ADDR_T PagingDescend(
	IN PHYS_ADDR_T TablePhys,
	IN SIZE_T Index,
	IN BOOLEAN Create
) {
	UINT64 *Table = (UINT64 *)MmPhysToVirt(TablePhys);
	UINT64  Entry = Table[Index];

	if (Entry & PTE_PRESENT) {
		if (Entry & PTE_PAGE_SIZE) {
			/* 
				* A 1 GiB or 2 MiB leaf. Splitting it is a real operation with
				* real TLB implications; refusing loudly beats silently
				* corrupting the mapping.
			*/
			return PHYS_ADDR_INVALID;
		}
		return (PHYS_ADDR_T)(Entry & PTE_ADDR_MASK);
	}
 
	if (!Create) {
		return PHYS_ADDR_INVALID;
	}

	PHYS_ADDR_T Fresh = MmAllocatePage();
	if (Fresh == PHYS_ADDR_INVALID) {
		return PHYS_ADDR_INVALID;
	}
 
	MemSet(MmPhysToVirt(Fresh), 0, PAGE_SIZE);
 
	/*
		* Intermediate entries stay permissive (present + writable + user);
		* the leaf is what actually decides access. Restricting here would
		* silently clamp every mapping underneath.
	*/

	Table[Index] = (UINT64)Fresh | PTE_PRESENT | PTE_WRITABLE | PTE_USER;
 
	return Fresh;
}

/* ==========================================================================
 * Mapping
 * ======================================================================= */
SHSTATUS MmMapPage(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN PHYS_ADDR_T Phys,
	IN _MM_PROTECTION Prot
) {
	PHYS_ADDR_T Pdpt = PagingDescend(Space->TopLevelTable, PML4_INDEX(Virt), TRUE);
	if (Pdpt == PHYS_ADDR_INVALID) {
		return (SHSTATUS)-1;
	}
 
	PHYS_ADDR_T Pd = PagingDescend(Pdpt, PDPT_INDEX(Virt), TRUE);
	if (Pd == PHYS_ADDR_INVALID) {
		return (SHSTATUS)-1;
	}
 
	PHYS_ADDR_T Pt = PagingDescend(Pd, PD_INDEX(Virt), TRUE);
	if (Pt == PHYS_ADDR_INVALID) {
		return (SHSTATUS)-1;
	}
 
	UINT64 *Table = (UINT64 *)MmPhysToVirt(Pt);
	Table[PT_INDEX(Virt)] = ((UINT64)Phys & PTE_ADDR_MASK) | PagingProtToFlags(Prot, TRUE);
 
	MmInvalidatePage(Virt);
	return STATUS_SUCCESS;
}

SHSTATUS MmMapHugePage(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt,
	IN PHYS_ADDR_T Phys,
	IN _MM_PROTECTION Prot
) {
	PHYS_ADDR_T Pdpt = PagingDescend(Space->TopLevelTable,
									 PML4_INDEX(Virt), TRUE);
	if (Pdpt == PHYS_ADDR_INVALID) {
		return (SHSTATUS)-1;
	}
 
	UINT64 *Table = (UINT64 *)MmPhysToVirt(Pdpt);
	Table[PDPT_INDEX(Virt)] = ((UINT64)Phys & PTE_ADDR_MASK) |
							  PagingProtToFlags(Prot, FALSE) |
							  PTE_PAGE_SIZE;
 
	MmInvalidatePage(Virt);
	return STATUS_SUCCESS;
}

VOID MmUnmapPage(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt
) {
	PHYS_ADDR_T Pdpt = PagingDescend(Space->TopLevelTable,
									 PML4_INDEX(Virt), FALSE);
	if (Pdpt == PHYS_ADDR_INVALID) {
		return;
	}
 
	PHYS_ADDR_T Pd = PagingDescend(Pdpt, PDPT_INDEX(Virt), FALSE);
	if (Pd == PHYS_ADDR_INVALID) {
		return;
	}
 
	PHYS_ADDR_T Pt = PagingDescend(Pd, PD_INDEX(Virt), FALSE);
	if (Pt == PHYS_ADDR_INVALID) {
		return;
	}
 
	UINT64 *Table = (UINT64 *)MmPhysToVirt(Pt);
	Table[PT_INDEX(Virt)] = 0;
 
	/*
		* Intermediate tables are deliberately not freed when they empty
		* out. Reclaiming them needs a per-table occupancy count, and an
		* address space that churns mappings would thrash without one.
		* Worth doing when something actually unmaps in bulk.
	*/
 
	MmInvalidatePage(Virt);
}
 
PHYS_ADDR_T MmGetPhysicalAddress(
	IN _PMM_ADDRESS_SPACE Space,
	IN VIRT_ADDR_T Virt
) {
	UINT64 *Table = (UINT64 *)MmPhysToVirt(Space->TopLevelTable);
	UINT64  Entry = Table[PML4_INDEX(Virt)];
 
	if (!(Entry & PTE_PRESENT)) {
		return PHYS_ADDR_INVALID;
	}
 
	Table = (UINT64 *)MmPhysToVirt(Entry & PTE_ADDR_MASK);
	Entry = Table[PDPT_INDEX(Virt)];
 
	if (!(Entry & PTE_PRESENT)) {
		return PHYS_ADDR_INVALID;
	}
 
	if (Entry & PTE_PAGE_SIZE) {                    /* 1 GiB leaf */
		return (PHYS_ADDR_T)((Entry & PTE_ADDR_MASK) +
							 (Virt & (HUGE_1G_SIZE - 1)));
	}
 
	Table = (UINT64 *)MmPhysToVirt(Entry & PTE_ADDR_MASK);
	Entry = Table[PD_INDEX(Virt)];
 
	if (!(Entry & PTE_PRESENT)) {
		return PHYS_ADDR_INVALID;
	}
 
	if (Entry & PTE_PAGE_SIZE) {                    /* 2 MiB leaf */
		return (PHYS_ADDR_T)((Entry & PTE_ADDR_MASK) +
							 (Virt & (HUGE_2M_SIZE - 1)));
	}
 
	Table = (UINT64 *)MmPhysToVirt(Entry & PTE_ADDR_MASK);
	Entry = Table[PT_INDEX(Virt)];
 
	if (!(Entry & PTE_PRESENT)) {
		return PHYS_ADDR_INVALID;
	}
 
	return (PHYS_ADDR_T)((Entry & PTE_ADDR_MASK) + (Virt & PAGE_MASK));
}
 
/* ==========================================================================
 * Kernel section mapping
 * ======================================================================= */
static VOID PagingMapKernelSection(
	IN PVOID Start,
	IN PVOID End,
	IN _MM_PROTECTION Prot,
	IN PCCHAR Label
) {
	(VOID)Label;
	VIRT_ADDR_T First = PAGE_ALIGN_DOWN((VIRT_ADDR_T)Start);
	VIRT_ADDR_T Last  = PAGE_ALIGN_UP((VIRT_ADDR_T)End);
 
	for (VIRT_ADDR_T Virt = First; Virt < Last; Virt += PAGE_SIZE) {
		PHYS_ADDR_T Phys = MmKernelVirtToPhys((PVOID)Virt);
 
		if (MmMapPage(&KernelSpace, Virt, Phys, (_MM_PROTECTION)(Prot | MM_PROT_GLOBAL)) != STATUS_SUCCESS) {
			return;
		}
	}
}
 
/* ==========================================================================
 * Initialization
 * ======================================================================= */
_PMM_ADDRESS_SPACE MmGetKernelAddressSpace(VOID) {
	return &KernelSpace;
}
 
SHSTATUS MmInitPaging(VOID) {
	_MM_STATS Stats;
	MmGetPhysicalStats(&Stats);
 
	/*
		* PAT first: every write-combining mapping made below depends on
		* slot 4 already meaning WC.
	*/
	PagingWriteMsr(MSR_IA32_PAT, PAT_CONFIGURATION);
 
	PHYS_ADDR_T Root = MmAllocatePage();
	if (Root == PHYS_ADDR_INVALID) {
		return (SHSTATUS)-1;
	}
 
	/*
		* Still on the boot stub's tables here, so MmPhysToVirt works
		* through its 512 GiB direct map -- which is why the stub maps that
		* much rather than the 4 GiB old Phonon used. A page-table frame
		* handed out above the stub's window would be unreachable at
		* exactly the moment we need to write it.
	*/
	MemSet(MmPhysToVirt(Root), 0, PAGE_SIZE);
 
	KernelSpace.TopLevelTable = Root;
	KernelSpace.MmioNext      = MM_MMIO_BASE;
	KernelSpace.HeapNext      = MM_HEAP_BASE;
 
	/* --- Direct map, sized to real RAM, 1 GiB leaves --- */
	PHYS_ADDR_T DirectLimit =
		(PHYS_ADDR_T)((Stats.HighestPhysical + HUGE_1G_SIZE - 1) &
					  ~(HUGE_1G_SIZE - 1));
 
	for (PHYS_ADDR_T Phys = 0; Phys < DirectLimit; Phys += HUGE_1G_SIZE) {
		VIRT_ADDR_T Virt = MM_DIRECT_MAP_BASE + Phys;
 
		if (MmMapHugePage(&KernelSpace, Virt, Phys, (_MM_PROTECTION)(MM_PROT_READ | MM_PROT_WRITE | MM_PROT_GLOBAL)) != STATUS_SUCCESS) {
			return (SHSTATUS)-1;
		}
	}
 
	/* --- Kernel image, real permissions, 4 KiB leaves ---
		* .text stays executable and read-only; everything else gets NX.
		* CR0.WP is already set by firmware, so the read-only mappings are
		* enforced even at CPL0 -- a stray write to .rodata faults instead
		* of quietly succeeding.
	*/
	PagingMapKernelSection(__TextStart, __TextEnd, (_MM_PROTECTION)(MM_PROT_READ | MM_PROT_EXECUTE), "text");
	PagingMapKernelSection(__RodataStart, __RodataEnd, (_MM_PROTECTION)(MM_PROT_READ), "rodata");
	PagingMapKernelSection(__DataStart, __DataEnd, (_MM_PROTECTION)(MM_PROT_READ | MM_PROT_WRITE), "data");
	PagingMapKernelSection(__BssStart, __BssEnd, (_MM_PROTECTION)(MM_PROT_READ | MM_PROT_WRITE), "bss");
	PagingMapKernelSection(__XScopeNodesStart, __XScopeNodesEnd, (_MM_PROTECTION)MM_PROT_READ, "xscope");
 
	/*
		* Note what is deliberately absent: any identity mapping. The boot
		* stub's PML4[0] does not exist in these tables, so every raw
		* physical pointer taken before this line dies here. The PMM's
		* bitmap pointer is one of them -- hence the rebase immediately
		* after the switch, with nothing in between that could touch it.
	*/
	MmSwitchAddressSpace(&KernelSpace);
	MmPmmRebaseToDirectMap();
 
	return STATUS_SUCCESS;
}

XSCOPENODE(MM_Paging, MmInitPaging, "MM_PMM");