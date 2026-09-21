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
#include <GAL/GOP.H>
#include <Lib/Lib.H>
#include <XAL/XScope.H>
#include <MM/MM.H>
#include <info.h>
#include <Int/IDT.H>
#include <IAL/IAL.H>
#include <IAL/PIC.H>
 
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
static VOID KernelRemapFramebuffer(VOID);
 
/* --- Functions ---*/
static VOID KernelRemapFramebuffer(VOID) {
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
}
 
VOID KernelMain(
	IN PPhononBootInfo Info
) {
	if (!Info || Info->magic != PHONON_BOOT_INFO_MAGIC) {
		for (;;) {
			__asm__ __volatile__("cli\n\thlt");
		}
	}
 
	MemCpy(&BootInfo, Info, sizeof(PhononBootInfo));
 
	// /* --- Memory --- */
	// if (MmInitPhysical(&BootInfo) != STATUS_SUCCESS) {
	// 	printk("[!] Physical memory init failed\n");
	// 	for (;;) {
	// 		__asm__ __volatile__("cli\n\thlt");
	// 	}
	// }
 
	// if (MmInitPaging() != STATUS_SUCCESS) {
	// 	printk("[!] Paging init failed\n");
	// 	for (;;) {
	// 		__asm__ __volatile__("cli\n\thlt");
	// 	}
	// }

	XScopeRun();
 
	/*
		* Dead zone: the identity map is gone and fb.Front still points into
		* it. No printk, no drawing, nothing that touches the framebuffer
		* until the remap below lands.
	*/
	KernelRemapFramebuffer();
	MmReclaimBootServices(&BootInfo);
	
	/* Output is safe again, and now write-combining. */
	_MM_STATS Stats;
	MmGetPhysicalStats(&Stats);


	/* --- Output, while the identity map still makes the framebuffer
		* reachable at its physical address.
	*/
	GALSetBackend(GAL_GOPBackend(&BootInfo));
	ConsoleInit();
	FbInit();
 
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

	IdtInit();

	IALSetBackend(IALPicBackend());
	if (IALInit() != 0) {
        printk("[!] IAL backend '%a' failed to initialize\n", IALBackendName());
        for (;;) { __asm__ __volatile__("cli\n\thlt"); }
    }
    printk("[Ial] Backend: %a\n", IALBackendName());

    __asm__ __volatile__("sti");

	for (;;) {
		__asm__ __volatile__("hlt");
	}
}