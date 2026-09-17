/*
	* Shadow/MM/PMM.C - [Enter description]
	* Author:   amity
	* Date:     Wed Sep 16 09:49:16 2026
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
	* Bitmap convention: 1 = in use / unavailable, 0 = free. Starting
	* all-ones means anything the firmware map never mentioned stays
	* unavailable by default, which is the safe direction to fail in.
*/

#include "Internal/Types.H"
#define BITMAP_WORD_BITS    64
#define BITMAP_WORD(Pfn)    ((Pfn) / BITMAP_WORD_BITS)
#define BITMAP_BIT(Pfn)     ((Pfn) % BITMAP_WORD_BITS)
 
/*
	* Never hand out the first megabyte: real-mode IVT, BDA, EBDA, VGA
	* window, SMM shadows. Some of it is technically free; none of it is
	* worth the class of bug it produces.
*/
#define LOW_MEMORY_CUTOFF   0x100000ULL 

/* --- Includes ---*/
#include <Lib/String.H>
#include <MM/MM.H>
#include <MM/PMM.H>
#include <info.h>

/* --- Typedefs - Structs - Enums ---*/

/*
	* UEFI's descriptor, redeclared here rather than pulled from EDK2:
	* Shadow must not depend on EDK2 headers, and this is the only place in
	* the kernel that ever parses one. If it ever needs a second consumer,
	* that is the signal to promote it into the boot-info layer instead of
	* copying it around.
*/
typedef struct EFI_MEMORY_DESCRIPTOR {
	UINT32 Type;
	UINT32 Pad;
	UINT64 PhysicalStart;
	UINT64 VirtualStart;
	UINT64 NumberOfPages;
	UINT64 Attribute;
} _EFI_MEMORY_DESCRIPTOR, *_PEFI_MEMORY_DESCRIPTOR;
 
typedef enum {
	EfiReservedMemoryType      = 0,
	EfiLoaderCode              = 1,
	EfiLoaderData              = 2,
	EfiBootServicesCode        = 3,
	EfiBootServicesData        = 4,
	EfiRuntimeServicesCode     = 5,
	EfiRuntimeServicesData     = 6,
	EfiConventionalMemory      = 7,
	EfiUnusableMemory          = 8,
	EfiACPIReclaimMemory       = 9,
	EfiACPIMemoryNVS           = 10,
	EfiMemoryMappedIO          = 11,
	EfiMemoryMappedIOPortSpace = 12,
	EfiPalCode                 = 13,
	EfiPersistentMemory        = 14,
} _EFI_MEMORY_TYPE;

/* --- Globals ---*/
static UINT64     *PmmBitmap;
static SIZE_T      PmmBitmapWords;
static SIZE_T      PmmTotalPages;
static PHYS_ADDR_T PmmBitmapPhys;
static SIZE_T      PmmFreePages;
static PHYS_ADDR_T PmmHighestPhysical;
static PFN_T       PmmSearchHint;

/* --- Prototypes ---*/
static BOOLEAN PmmTestBit(IN PFN_T Pfn);
static VOID    PmmSetBit(IN PFN_T Pfn);
static VOID    PmmClearBit(IN PFN_T Pfn);
static BOOLEAN PmmIsReclaimable(IN UINT32 Type);

/* --- Functions ---*/

/* ==========================================================================
 * Bitmap primitives
 * ======================================================================= */
static BOOLEAN PmmTestBit(
	IN PFN_T Pfn
) {
	if (Pfn >= PmmTotalPages) {
		return TRUE; /* Out of range reads as used */
	}

	return (PmmBitmap[BITMAP_WORD(Pfn)] >> BITMAP_BIT(Pfn)) & 1ULL;
}

static VOID PmmSetBit(
	IN PFN_T Pfn
) {
	if (Pfn >= PmmTotalPages) {
		return;
	}

	if (!PmmTestBit(Pfn)) {
		PmmFreePages--;
	}

	PmmBitmap[BITMAP_WORD(Pfn)] |= (1ULL << BITMAP_BIT(Pfn));
}

static VOID PmmClearBit(
	IN PFN_T Pfn
) {
		if (Pfn >= PmmTotalPages) {
		return;
	}
	if (PmmTestBit(Pfn)) {
		PmmFreePages++;
	}
	PmmBitmap[BITMAP_WORD(Pfn)] &= ~(1ULL << BITMAP_BIT(Pfn));
}

/* ==========================================================================
 * Which UEFI memory types may we take?
 *
 * ONLY EfiConventionalMemory, for now, and the omissions are deliberate:
 *
 *   EfiLoaderCode/Data  - holds the running kernel image (ElfLoadImage
 *                         allocated it as EfiLoaderData) AND the memory
 *                         map buffer we are walking right now. Reclaiming
 *                         it frees the ground under our own feet.
 *   EfiBootServicesCode/Data
 *                       - spec says reclaimable after ExitBootServices,
 *                         but firmware's page tables live here and we are
 *                         still executing on them until MmInitPaging()
 *                         switches CR3. Reclaimable LATER, not now.
 *   EfiACPIReclaimMemory
 *                       - holds the ACPI tables. Not reclaimable until
 *                         ACPICA has finished parsing them, which is many
 *                         subsystems away.
 *   EfiRuntimeServices* - firmware still owns these for the machine's
 *                         entire life. Never ours.
 *
 * Each of those is a later, separate, deliberate decision. None of them
 * is a free win worth taking on day one.
 * ======================================================================= */

static BOOLEAN PmmIsReclaimable(
	IN UINT32 Type
) {
	return (Type == EfiConventionalMemory);
}

/*
	* Does this descriptor type describe actual RAM? EfiMemoryMappedIO and
	* EfiMemoryMappedIOPortSpace describe address-space WINDOWS for devices
	* -- a 64-bit PCIe aperture here can be terabytes wide on real hardware,
	* and sizing the bitmap for it wastes real memory on tracking frames
	* that will never exist. This is the ONLY thing that decides bitmap
	* range; PmmIsReclaimable (a stricter subset) still decides what gets
	* freed.
*/

static BOOLEAN PmmIsRamBacked(
	IN UINT32 Type
) {
    switch (Type) {
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
        case EfiACPIReclaimMemory:
        case EfiACPIMemoryNVS:
        case EfiPersistentMemory:
            return TRUE;
        default:
            return FALSE;
    }
}

/* ==========================================================================
 * Initialization
 *
 * Still running on firmware's identity map: every address in this
 * function is physical AND dereferenceable. MmPhysToVirt() would be
 * wrong here and is deliberately not used.
 * ======================================================================= */

SHSTATUS MmInitPhysical(
	IN PPhononBootInfo Info
) {
	if (!Info || Info->magic != PHONON_BOOT_INFO_MAGIC) {
		return (SHSTATUS)-1;
	}

	UINT8 *MapBase    = (UINT8 *)(UINT_PTR)Info->memmap_base;
	SIZE_T EntryCount = (SIZE_T)Info->memmap_entry_count;
	SIZE_T EntrySize  = (SIZE_T)Info->memmap_entry_size;

	/* 
		* Pass 1: how much physical address space is there?
		* Stride by memmap_entry_size, NEVER sizeof(_EFI_MEMORY_DESCRIPTOR).
		* The spec explicitly allows firmware to return a larger stride, and
		* assuming otherwise walks straight off into garbage.
	*/

	PmmHighestPhysical = 0;

	for (SIZE_T i = 0; i < EntryCount; i++) {
		_PEFI_MEMORY_DESCRIPTOR Desc = (_PEFI_MEMORY_DESCRIPTOR)(MapBase + (i * EntrySize));

		if (!PmmIsRamBacked(Desc->Type)) {
			continue;
		}

		PHYS_ADDR_T End = Desc->PhysicalStart + (Desc->NumberOfPages * PAGE_SIZE);

		if (End > PmmHighestPhysical) {
			PmmHighestPhysical = End;
		}
	}

	PmmTotalPages  = (SIZE_T)(PmmHighestPhysical >> PAGE_SHIFT);
	PmmBitmapWords = (PmmTotalPages + BITMAP_WORD_BITS - 1) / BITMAP_WORD_BITS;

	SIZE_T BitmapBytes = PmmBitmapWords * sizeof(UINT64);

	/*
		* Pass 2: find somewhere to put the bitmap
		* No allocator exists yet, so the bitmap has to be placed by hand.
		* First conventional region large enough wins. Above the low-memory
		* cutoff so we never land the bitmap somewhere we have decided is
		* untouchable.
	*/

	PHYS_ADDR_T BitmapPhys = PHYS_ADDR_INVALID;

	for (SIZE_T i = 0; i < EntryCount; i++) {
		_PEFI_MEMORY_DESCRIPTOR Desc = (_PEFI_MEMORY_DESCRIPTOR)(MapBase + (i * EntrySize));

		if (!PmmIsReclaimable(Desc->Type)) {
			continue;
		}

		PHYS_ADDR_T Start = Desc->PhysicalStart;
		SIZE_T      Bytes = (SIZE_T)(Desc->NumberOfPages * PAGE_SIZE);

		if (Start < LOW_MEMORY_CUTOFF) {
			if (Start + Bytes <= LOW_MEMORY_CUTOFF) {
				continue;
			}

			Bytes -= (SIZE_T)(LOW_MEMORY_CUTOFF - Start);
			Start  = LOW_MEMORY_CUTOFF;
		}

		if (Bytes > BitmapBytes) {
			BitmapPhys = PAGE_ALIGN_UP(Start);
			break;
		}
	}

	if (BitmapPhys == PHYS_ADDR_INVALID) {
		return (SHSTATUS)-1;
	}

	PmmBitmapPhys = BitmapPhys;
	PmmBitmap = (UINT64 *)(UINT_PTR)BitmapPhys;

	/*
		* Everything starts unavailable. Regions the firmware never described
		* stay that way — better to lose usable RAM than to hand out a page
		* that belongs to a device.
	*/

	MemSet(PmmBitmap, 0xFF, BitmapBytes);
	PmmFreePages = 0;

	/* Pass 3: release what is genuinely ours */
	for (SIZE_T i = 0; i < EntryCount; i++) {
		_PEFI_MEMORY_DESCRIPTOR Desc =
			(_PEFI_MEMORY_DESCRIPTOR)(MapBase + (i * EntrySize));
 
		if (!PmmIsReclaimable(Desc->Type)) {
			continue;
		}
 
		PFN_T First = PFN_OF(Desc->PhysicalStart);
		PFN_T Last  = First + (PFN_T)Desc->NumberOfPages;
 
		for (PFN_T Pfn = First; Pfn < Last; Pfn++) {
			PmmClearBit(Pfn);
		}
	}

	/* Take back what we must not give away */
 
	/* Page zero: a NULL dereference should fault, not quietly succeed. */

	PmmSetBit(0);
 
	/* Low memory. */

	MmReservePhysicalRange(0, LOW_MEMORY_CUTOFF);
 
	/*
		* The bitmap's own storage — it was sitting in conventional memory
		* and pass 3 just marked it free. It is not.
	*/

	MmReservePhysicalRange(BitmapPhys, BitmapBytes);
 
	/*
		* The memory map buffer itself: EfiLoaderData, so never reclaimed —
		* but reserving it explicitly means this stays correct even if the
		* reclaimable set widens later.
	*/

	MmReservePhysicalRange(Info->memmap_base, EntryCount * EntrySize);
 
	/* 
		* The framebuffer. Firmware may describe it as EfiMemoryMappedIO, or
		* may not describe it at all — neither is something to rely on.
	*/
	if (Info->framebuffer_base != 0) {
		MmReservePhysicalRange(Info->framebuffer_base, (SIZE_T)Info->framebuffer_pitch * Info->framebuffer_height);
	}

	PmmSearchHint = PFN_OF(LOW_MEMORY_CUTOFF);
 
	return STATUS_SUCCESS;
}

/* ==========================================================================
 * Allocation
 * ======================================================================= */

PHYS_ADDR_T MmAllocatePage(VOID) {
	return MmAllocatePages(1);
}

PHYS_ADDR_T MmAllocatePages(
	IN SIZE_T Count
) {
	if (Count == 0 || Count > PmmFreePages) {
		return PHYS_ADDR_INVALID;
	}
 
	/* 
		* Two sweeps: from the hint to the end, then from the bottom to the
		* hint. Allocation is overwhelmingly sequential in practice, so the
		* hint turns most calls into a single-word test.
	*/
	for (INT Sweep = 0; Sweep < 2; Sweep++) {
		PFN_T Pfn   = (Sweep == 0) ? PmmSearchHint : PFN_OF(LOW_MEMORY_CUTOFF);
		PFN_T Limit = (Sweep == 0) ? PmmTotalPages : PmmSearchHint;
 
		while (Pfn < Limit) {
			/* Skip whole words that are entirely in use. */
			if (BITMAP_BIT(Pfn) == 0 &&
				PmmBitmap[BITMAP_WORD(Pfn)] == ~0ULL) {
				Pfn += BITMAP_WORD_BITS;
				continue;
			}
 
			if (PmmTestBit(Pfn)) {
				Pfn++;
				continue;
			}
 
			/* Candidate start — is the whole run free? */
			SIZE_T Run = 0;
			while (Run < Count && (Pfn + Run) < Limit && !PmmTestBit(Pfn + Run)) {
				Run++;
			}
 
			if (Run == Count) {
				for (SIZE_T i = 0; i < Count; i++) {
					PmmSetBit(Pfn + i);
				}
 
				PmmSearchHint = Pfn + Count;
				if (PmmSearchHint >= PmmTotalPages) {
					PmmSearchHint = PFN_OF(LOW_MEMORY_CUTOFF);
				}
 
				return PHYS_OF(Pfn);
			}
 
			/* Run broke at Pfn + Run; nothing before that can work. */
			Pfn += (Run + 1);
		}
	}
 
	return PHYS_ADDR_INVALID;
}

VOID MmFreePage(IN PHYS_ADDR_T Phys) {
	MmFreePages(Phys, 1);
}
 
VOID MmFreePages(IN PHYS_ADDR_T Phys, IN SIZE_T Count) {
	PFN_T First = PFN_OF(PAGE_ALIGN_DOWN(Phys));
 
	for (SIZE_T i = 0; i < Count; i++) {
		PmmClearBit(First + i);
	}
 
	if (First < PmmSearchHint) {
		PmmSearchHint = First;
	}
}

/* ==========================================================================
 * Reservation
 * ======================================================================= */

 VOID MmReservePhysicalRange(
	IN PHYS_ADDR_T Base,
	IN SIZE_T Bytes
) {
	if (Bytes == 0) {
		return;
	}
 
	PFN_T First = PFN_OF(PAGE_ALIGN_DOWN(Base));
	PFN_T Last  = PFN_OF(PAGE_ALIGN_UP(Base + Bytes));
 
	for (PFN_T Pfn = First; Pfn < Last; Pfn++) {
		PmmSetBit(Pfn);
	}
}
 
/* ==========================================================================
 * Introspection
 * ======================================================================= */
VOID MmGetPhysicalStats(
	OUT _PMM_STATS Stats
) {
	if (!Stats) {
		return;
	}
 
	Stats->TotalPages      = PmmTotalPages;
	Stats->FreePages       = PmmFreePages;
	Stats->UsedPages       = PmmTotalPages - PmmFreePages;
	Stats->ReservedPages   = PmmTotalPages - PmmFreePages;
	Stats->HighestPhysical = PmmHighestPhysical;
}

VOID MmPmmRebaseToDirectMap(VOID) {
    PmmBitmap = (UINT64 *)MmPhysToVirt(PmmBitmapPhys);
}

VOID MmReclaimBootServices(IN PPhononBootInfo Info) {
    /*
		* Info->memmap_base is a physical address captured while the
		* identity map was still live -- same situation the PMM bitmap was
		* in, same fix. This is exactly why MmInitPhysical reserved this
		* range instead of just leaving it alone: the bytes are still there,
		* just not reachable at that address anymore.
    */
	
    UINT8 *MapBase     = (UINT8 *)MmPhysToVirt(Info->memmap_base);
    SIZE_T EntryCount  = (SIZE_T)Info->memmap_entry_count;
    SIZE_T EntryStride = (SIZE_T)Info->memmap_entry_size;

    for (SIZE_T i = 0; i < EntryCount; i++) {
        _PEFI_MEMORY_DESCRIPTOR Desc =
            (_PEFI_MEMORY_DESCRIPTOR)(MapBase + (i * EntryStride));

        if (Desc->Type != EfiBootServicesCode &&
            Desc->Type != EfiBootServicesData) {
            continue;
        }

        PFN_T First = PFN_OF(Desc->PhysicalStart);
        PFN_T Last  = First + (PFN_T)Desc->NumberOfPages;

        for (PFN_T Pfn = First; Pfn < Last; Pfn++) {
            PmmClearBit(Pfn);
        }
    }
}