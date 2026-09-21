/*
	* Shadow/MM/Heap.C - Kernel pool allocator
	* Author:   amity
	* Date:     Thu Sep 17 08:52:40 2026
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
#define HEAP_ALIGNMENT   16
#define HEAP_MIN_CHUNK   0x100000ULL        /* 1 MiB per growth, minimum */
#define HEAP_MIN_SPLIT   32                 /* below this, don't bother splitting */
#define HEAP_BLOCK_MAGIC 0x504F4F4CU        /* "POOL" */
 
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((SIZE_T)(a) - 1))

/* --- Includes ---*/
#include <Lib/Lib.H>
#include <MM/MM.H>
#include <XAL/XScope.H>

/* --- Typedefs - Structs - Enums ---*/

/*
	* One list, address-ordered, holding both used and free blocks. A
	* block's Next/Prev are its neighbors in the list AND (usually -- see
	* the adjacency check in ExFreePool) in memory, which is what makes
	* coalescing a pointer comparison instead of a search.
*/

typedef struct HEAP_BLOCK {
	SIZE_T Size; /* usable size, excludes this header */
	BOOLEAN Free;
	UINT32 Magic;
	struct HEAP_BLOCK *Next;
	struct HEAP_BLOCK *Prev;
} _HEAP_BLOCK, *_PHEAP_BLOCK;

/* --- Globals ---*/
static BOOLEAN HeapReady = FALSE;

static _PHEAP_BLOCK HeapHead = NULL;
static _PHEAP_BLOCK HeapTail = NULL;

/* --- Prototypes ---*/
static SHSTATUS 	HeapExpand(IN SIZE_T MinBytes);
static _PHEAP_BLOCK HeapFindFit(IN SIZE_T Bytes);
static VOID         HeapSplit(IN _PHEAP_BLOCK Block, IN SIZE_T Bytes);

/* --- Functions ---*/

/* ==========================================================================
 * Growth
 *
 * New chunks are always appended at the tail: MmAllocateVirtual is a bump
 * allocator that never reuses an address, so a fresh chunk is always at
 * a higher virtual address than everything before it. That is what lets
 * this stay a plain append rather than an insertion search.
 * ======================================================================= */

static SHSTATUS HeapExpand(
	IN SIZE_T MinBytes
) {
	SIZE_T ChunkBytes = MinBytes + sizeof(_HEAP_BLOCK);

	if (ChunkBytes < HEAP_MIN_CHUNK) {
		ChunkBytes = HEAP_MIN_CHUNK;
	}

	ChunkBytes = PAGE_ALIGN_UP(ChunkBytes);

	VIRT_ADDR_T Virt = MmAllocateVirtual(MmGetKernelAddressSpace(), ChunkBytes, (_MM_PROTECTION)(MM_PROT_READ | MM_PROT_WRITE));

	if (Virt == MM_VIRT_INVALID) {
		return (SHSTATUS)-1;
	}

	_PHEAP_BLOCK NewBlock = (_PHEAP_BLOCK)Virt;
	NewBlock->Size  = ChunkBytes - sizeof(_HEAP_BLOCK);
	NewBlock->Free  = TRUE;
	NewBlock->Magic = HEAP_BLOCK_MAGIC;
	NewBlock->Prev  = HeapTail;
	NewBlock->Next  = NULL;
 
	if (HeapTail) {
		HeapTail->Next = NewBlock;
	} else {
		HeapHead = NewBlock;
	}
	HeapTail = NewBlock;
 
	return STATUS_SUCCESS;
}

/* ==========================================================================
 * Allocation
 * ======================================================================= */
static _PHEAP_BLOCK HeapFindFit(
	IN SIZE_T Bytes
) {
	for (_PHEAP_BLOCK Block = HeapHead; Block; Block = Block->Next) {
		if (Block->Free && Block->Size >= Bytes) {
			return Block;
		}
	}

	return NULL;
}

static VOID HeapSplit(
	IN _PHEAP_BLOCK Block,
	IN SIZE_T Bytes
) {
	if (Block->Size < Bytes + sizeof(_HEAP_BLOCK) + HEAP_MIN_SPLIT) {
		/*
			* Remainder too small to be worth its own header -- the caller
			* just gets a few extra bytes of slack instead.
		*/

		return;
	}

	_PHEAP_BLOCK NewBlock = (_PHEAP_BLOCK)((UINT8 *)Block + sizeof(_HEAP_BLOCK) + Bytes);

	NewBlock->Size  = Block->Size - Bytes - sizeof(_HEAP_BLOCK);
	NewBlock->Free  = TRUE;
	NewBlock->Magic = HEAP_BLOCK_MAGIC;
	NewBlock->Prev  = Block;
	NewBlock->Next  = Block->Next;

	if (Block->Next) {
		Block->Next->Prev = NewBlock;
	} else {
		Block->Next = NewBlock;
	}

	Block->Next = NewBlock;
	Block->Size = Bytes;
}

/* Malloc? No, unline Unix I'm not scared of words */
PVOID ExAllocatePool(
	IN SIZE_T Bytes
) {
	if (Bytes == 0) {
		return NULL;
	}

	_PHEAP_BLOCK Block = HeapFindFit(Bytes);

	if (!Block) {
		if (HeapExpand(Bytes) != STATUS_SUCCESS) {
			return NULL;
		}

		Block = HeapFindFit(Bytes);
		if (!Block) {
			/* 
				* HeapExpand reported success but nothing fits: should not
				* happen given how ChunkBytes is sized, but returning NULL
				* beats indexing a block that isn't there.
			*/

			return NULL;
		}
	}

	HeapSplit(Block, Bytes);
	Block->Free = FALSE; 

	return (PVOID)((UINT8 *)Block + sizeof(_HEAP_BLOCK));
}

/* Calloc but cool */
PVOID ExAllocatePoolZeroed(
	IN SIZE_T Bytes
) {
	PVOID Ptr = ExAllocatePool(Bytes);

	if (Ptr) {
		MemSet(Ptr, 0, Bytes);
	}

	return Ptr;
}

/* It's free, I don't know what to say, can't argue with peak */
VOID ExFreePool(
	IN PVOID Ptr
) {
	if (!Ptr) {
		return;
	}

	_PHEAP_BLOCK Block = (_PHEAP_BLOCK)((UINT8 *)Ptr - sizeof(_HEAP_BLOCK));

	if (Block->Magic != HEAP_BLOCK_MAGIC) {
		printk("[Heap] ExFreePool(%p): bad magic -- invalid pointer or heap corruption, refusing to free\n", Ptr);
		return;
	}

	if (Block->Free) {
		printk("[Heap] ExFreePool(%p): double free\n", Ptr);
		return;
	}

	Block->Free = TRUE;

	/* 
		* Forward coalesce. The adjacency check is the part that matters;
		* Block->Next being free is not sufficient by itself, see the
		* chunk-boundary/guard-page note above HeapExpand.
	*/

	if (Block->Next && Block->Next->Free && (UINT8 *)Block + sizeof(_HEAP_BLOCK) + Block->Size == (UINT8 *)Block->Next) {
		_PHEAP_BLOCK Dead = Block->Next;

		Block->Size += sizeof(_HEAP_BLOCK) + Dead->Size;
		Block->Next = Dead->Next;

		if (Dead->Next) {
			Dead->Next->Prev = Block;
		} else {
			HeapTail = Block;
		}
	}

	/* Backward coalesce, same adjacency requirement. */
	if (Block->Prev && Block->Prev->Free && (UINT8 *)Block->Prev + sizeof(_HEAP_BLOCK) + Block->Prev->Size == (UINT8 *)Block) {
		_PHEAP_BLOCK Prev = Block->Prev;
 
		Prev->Size += sizeof(_HEAP_BLOCK) + Block->Size;
		Prev->Next  = Block->Next;
 
		if (Block->Next) {
			Block->Next->Prev = Prev;
		} else {
			HeapTail = Prev;
		}
	}
}

/* It's a mallocator, innit? 🦄*/
SHSTATUS ExInitializePool(VOID) {
	if (HeapExpand(HEAP_MIN_CHUNK) != STATUS_SUCCESS) {
		return (SHSTATUS)-1;
	}

	HeapReady = TRUE;
	return STATUS_SUCCESS;
}

BOOLEAN ExPoolReady(VOID) {
	return HeapReady;
}

XSCOPENODE(MM_Heap, ExInitializePool, "MM_VMM");