/*
	* Shadow/GAL/GOP.C - GAL backend for UEFI GOP framebuffer
	* Author:   amity
	* Date:     Mon Sep 14 12:40:01 2026
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

/* --- Includes ---*/
#include <GAL/GOP.H>
#include <info.h>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static PPhononBootInfo boot_info;
static VIRT_ADDR_T GopVirtualBase = 0;

/* --- Prototypes ---*/

/* --- Functions ---*/
static INT GAL_GOPInit(
	IN _GAL_BACKEND *Self
) {
	(VOID)Self;

	if (!boot_info || boot_info->magic != PHONON_BOOT_INFO_MAGIC) {
		return -1;
	}

	if (boot_info->framebuffer_base == 0) {
		return -1;
	}

	return 0;
}

static VOID GAL_GOPGetMode(
	IN _GAL_BACKEND *Self,
	OUT _GAL_MODE *Out
) {
	(VOID)Self;

	Out->Width  = boot_info->framebuffer_width;
	Out->Height = boot_info->framebuffer_height;
	Out->Pitch  = boot_info->framebuffer_pitch;
	Out->Bpp    = 32; /* only 32 bpp modes allowed, see Boot/Graphics.c */

	if (boot_info->framebuffer_format == PHONON_PIXEL_FORMAT_RGB8) {
		/*
		 * I like red, it's my favourite color
		 * Probably
		 * I'm colorblind so who knows!
		*/
		Out->RedMask   = 0x00FF0000;
		/*
		 * Green has the same position across RGB and BGR
		 * It would therefore make sense to pull it out the conditional
		 * But this is valid for these *two specific* formats
		 * Might not holf for other formats 
		*/
		Out->GreenMask = 0x0000FF00;
		/*
		 * x86_64 is little endian, the lowest bit, representing blue, should therefore
		 * make it BGR, but since Intel didn't invent color we ended up with this inconsistent mess
		 * Thanks artitst
		*/
		Out->BlueMask  = 0x000000FF;

		Out->RedPos   = 16;
		Out->GreenPos = 8;
		Out->BluePos  = 0;
		/*
		 * Insert other modes here, in an ELIF
		*/
	} else if (boot_info->framebuffer_format == PHONON_PIXEL_FORMAT_BGR8) {
		/*
		 * The bootlooader, only gives us RGB8 or BRG8 right now,
		 * I should know, I made the damn thing
		 * This isn't safe as fallback in an else,
		 * I know that it is valid as else, since there's only two possibilities
		 * But tell that to the VHDL compiler, 16 cases for a 3-bit binary number
		 * Obviously missing ~6.5K cases
		 * 
		 * Anyway, I'm feeling slightly better than usualm yay Amity
		*/

		Out->RedMask   = 0x000000FF;
		Out->GreenMask = 0x0000FF00;
		Out->BlueMask  = 0x00FF0000;

		Out->RedPos   = 0;
		Out->GreenPos = 8; /* See, it's the same, still not safe as an else */
		Out->BluePos  = 16;
	}
}

static PVOID GAL_GOPGetFrameBuffer(
	/*
	 * Should I increase the P-ness?
	 * P_GAL_BACKEND Self? 
	 * _PGAL_BACKEND Self?
	 * The second, I'll be back in a minute.
	 * Billy was onto something,
	 * or they couldn't figure out how to turn off caps lock
	*/

	IN _PGAL_BACKEND Self
) {
	(VOID)Self;

	/* Physical == virtual right now; Shadow has no page tables of its own yet
	 * This needs a real virtual mapping once that changes, not before.
	 * Note to self, make the Linker emit some symbols and implement an AutoVirt 
	 * and just care about this later
	 * Good plan, me!
	*/

	if (GopVirtualBase != 0) {
        return (PVOID)GopVirtualBase;
    }
    return (PVOID)(UINT_PTR)boot_info->framebuffer_base;
}

static VOID GAL_GOPPresent(
	IN _PGAL_BACKEND Self,
	IN const _PGFX_SURFACE Back
) {
	/*
	 * Seems useless, I know
	 * And it is
	 * But it's for consistency and most importantly:
	 * ✨ this plugged into an Abstraction Layer, GAL ✨
	 * VERY important: 🦄
	 * YESSS, there are unicorns
	 * Apparently emojis are a sign of code being AI generated
	 * 🦄✨I AM A GIRL, LET ME ENJOY UNICORNS AND SPARKLES ✨🦄
	*/

	(VOID)Self;
	(VOID)Back;
	/* A plain linear framebuffer needs nothing beyond the generic
   	 * backbuffer copy gal_present()'s caller already does. 
	 * No FIFO kick, no mode register writes. A backend that DOES need something extra
     * here (an SVGA-II GPU, for example) puts it in its own
     * present(), not here
	*/
}

static _GAL_BACKEND GAL_GOP = {
	.Name = "UEFI Graphics Output Protocol",
	.Init = GAL_GOPInit,
	.GetMode = GAL_GOPGetMode,
	.GetFramebuffer = GAL_GOPGetFrameBuffer,
	.Present = GAL_GOPPresent,
};

_PGAL_BACKEND GAL_GOPBackend(
	/*
	 * Oh sorry, you don't like P-notation?
	 * Too bad, I love it 🦄
	*/
	PPhononBootInfo Info
) {
	boot_info = Info;
	return &GAL_GOP;
}

VOID GAL_GOPSetVirtualBase(
	IN VIRT_ADDR_T Virt
) {
    GopVirtualBase = Virt;
}