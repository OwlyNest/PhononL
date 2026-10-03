/*
 * Shadow/Kernel.c - minimal kernel entry point
 *
 * Reads PhononBootInfo directly (SysV AMD64: arrives in %rdi, which the
 * compiler binds to this parameter automatically) rather than assuming
 * hardcoded struct offsets — the whole point of sharing info.h between
 * the bootloader and Shadow is that the compiler enforces agreement on
 * layout, not that both sides have to independently get it right.
 */
/* --- Macros ---*/
 
/* --- Includes ---*/
#include <GFX/GFX.H>
#include <GAL/GAL.H>
#include <Lib/Lib.H>
#include <XAL/XScope.H>
#include <MM/MM.H>
#include <DRV/PS2/PS2.H>
#include <info.h>
#include <Int/Int.H>
#include <IAL/IAL.H>
#include <TAL/TAL.H>
#include <BUS/PCI/PCI.H>

/* --- Typedefs - Structs - Enums ---*/
 
/* --- Globals ---*/
 
/*
	* The pointer PhononBoot hands us aims at a local in UefiMain, sitting
	* on firmware's stack. It survives only because nothing reclaims
	* EfiBootServicesData yet. Copy it into .bss before trusting it with
	* anything, so widening the reclaimable set later cannot quietly
	* poison the boot info.
*/
PhononBootInfo BootInfo;

/* --- Prototypes ---*/
/* --- Functions ---*/

VOID KernelMain(
	IN PPhononBootInfo Info
) {
	if (!Info || Info->magic != PHONON_BOOT_INFO_MAGIC) {
		for (;;) {
			__asm__ __volatile__("cli\n\thlt");
		}
	}
 
	MemCpy(&BootInfo, Info, sizeof(PhononBootInfo));
 

	XScopeRun();
 
	/*
		* Dead zone: the identity map is gone and fb.Front still points into
		* it. No printk, no drawing, nothing that touches the framebuffer
		* until the remap below lands.
	*/
	MmReclaimBootServices(&BootInfo);
	
	/* Output is safe again, and now write-combining. */
	_MM_STATS Stats;
	MmGetPhysicalStats(&Stats);
 
	printk("Shadow\r\n");
	printk("[x] Boot info v%u, framebuffer %ux%u\r\n",
		   BootInfo.version,
		   BootInfo.framebuffer_width,
		   BootInfo.framebuffer_height);

	printk("[x] Higher half live. %llu MiB free of %llu MiB\r\n",
		   (UINT64)((Stats.FreePages * PAGE_SIZE) / (1024 * 1024)),
		   (UINT64)((Stats.TotalPages * PAGE_SIZE) / (1024 * 1024)));

 
	if (ExPoolReady()) {
		printk("[x] Heap Heap Hooray!\r\n");
	}


	TALSetFrequency(1000);

	Kbr();

	CpuidDump();

	Kbr();

	PciInit();

	Kbr();


	for (;;) {
	}
}
