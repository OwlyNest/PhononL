/*
 * Shadow/Kernel.c - minimal kernel entry point
 *
 * Reads PhononBootInfo directly (SysV AMD64: arrives in %rdi, which the
 * compiler binds to this parameter automatically) rather than assuming
 * hardcoded struct offsets — the whole point of sharing info.h between
 * the bootloader and Shadow is that the compiler enforces agreement on
 * layout, not that both sides have to independently get it right.
 */

#include <info.h>
#include <GAL/GAL.H>
#include <GAL/GOP.H>
#include <GFX/FB.H>
#include <GFX/Console.H>
#include <Lib/PrintK.H>

#include <info.h>

VOID _start(PPhononBootInfo Info) {
	if (Info->magic != PHONON_BOOT_INFO_MAGIC) {
		// Can't trust anything else in Info if this doesn't match — no
		// framebuffer to draw to safely, nothing to do but halt.
		for (;;) {
		__asm__ __volatile__("cli\n\thlt");
		}
	}

	/* ------------------------------------------------------------------ */
	/* 1. Install the only backend we have right now (UEFI GOP)           */
	/* ------------------------------------------------------------------ */
	_PGAL_BACKEND GOP = GAL_GOPBackend(Info);
	GALSetBackend(GOP);

	/* ------------------------------------------------------------------ */
    /* 2. Bring up the framebuffer abstraction (back-buffer + mode info)  */
    /* ------------------------------------------------------------------ */
if (fb_init() != 0) {
        /* 
		 * Fallback: paint solid red directly so we still prove we are
         * in the kernel even if the higher layers failed.
		*/

        UINT32 *fb = (UINT32 *)(UINT_PTR)Info->framebuffer_base;
        UINT32 pitch = Info->framebuffer_pitch / 4;
        UINT32 total = pitch * Info->framebuffer_height;
        for (UINT32 i = 0; i < total; i++) {
            fb[i] = 0x00FF0000;
		}

        for (;;) {
            __asm__ __volatile__("cli\n\thlt");
		}
    }

	/* ------------------------------------------------------------------ */
    /* 3. Solid red background                                            */
    /* ------------------------------------------------------------------ */
    fb_clear(0x00FF0000);

	
	/* ------------------------------------------------------------------ */
    /* 4. Console + first printk                                          */
    /* ------------------------------------------------------------------ */
    ConsoleInit();
	printk("Shadow kernel online\r\n");
    printk("Backend : %s\r\n", GALBackendName());
    printk("Mode    : %ux%u @ %u bpp\r\n", fb.Back.Width, fb.Back.Height, 32);
    printk("Framebuf: %p\r\n", (PVOID)(UINT_PTR)Info->framebuffer_base);
    printk("Hello from printk on the red screen!\r\n");

    /* Force a present in case the last printk didn't already do it */
    if (ConsoleIsDirty()) {
        ConsoleRedraw();
	}
    fb_present();

	/* ------------------------------------------------------------------ */
    /* 5. Done – hang so the red + text stays visible                     */
    /* ------------------------------------------------------------------ */
	for (;;) {
		__asm__ __volatile__("cli\n\thlt");
	}
}