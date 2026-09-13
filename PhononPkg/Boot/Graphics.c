/*
 * Boot/Graphics.c - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:46:19 2026
 * Copyright © 2026 OwlyNest
 */

/* --- Styling Instructions ---
 * Encoding:                      UTF-8, Unix line endings
 * Text font:                     Monospace
 * Line width:                    Max 80 characters
 * Indentation:                   Use 4 spaces
 * Brace style:                   Same line as control statement
 * Inline comments:               Column 40, wherever possible, else, whole
 * multiple of 20 Section headers:               Use 3 '-' characters before and
 * after Pointer notation:              Next to variable name, not type Binary
 * operations:             Space around operator Empty parameter list: Use
 * (void) instead of () Statements and declarations:   Max one per line
 */

/* --- Macros ---*/

/* --- Includes ---*/
#include "AutoGen.h"
#include "Uefi/UefiBaseType.h"
#include "info.h"
#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/GraphicsOutput.h>

#include <Graphics.h>
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/
EFI_STATUS GraphicsInit(
    IN OUT PPhononBootInfo Info
) {
    EFI_STATUS                            Status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL         *Gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeInfo;

    Status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&Gop);
    if (EFI_ERROR(Status)) {
        Print(L"[!] LocateProtocol(GOP) failed: %r\n", Status);
        return Status;
    }

    // Using whatever mode firmware is already driving, rather than actively
    // switching modes. Getting a working, correctly-described framebuffer is
    // the milestone right now; picking a specific resolution (for the boot
    // animation, later) is a refinement on top of this, not a prerequisite.
    ModeInfo = Gop->Mode->Info;
    Info->framebuffer_base   = Gop->Mode->FrameBufferBase;
    Info->framebuffer_width  = ModeInfo->HorizontalResolution;
    Info->framebuffer_height = ModeInfo->VerticalResolution;
    Info->framebuffer_pitch  = ModeInfo->PixelsPerScanLine * 4; // 32bpp, both formats below

    switch (ModeInfo->PixelFormat) {
        case PixelBlueGreenRedReserved8BitPerColor: {
            Info->framebuffer_format = PHONON_PIXEL_FORMAT_BGR8;
            break;
        }
        case PixelRedGreenBlueReserved8BitPerColor: {
            Info->framebuffer_format = PHONON_PIXEL_FORMAT_RGB8;
            break;
        }
        default: {
            // PixelBitMask / PixelBltOnly: rare in practice on real hardware and
            // QEMU's stdvga/virtio-gpu GOP implementations, not handled yet.
            Print(L"[!] Unsupported GOP pixel format: %d\n", ModeInfo->PixelFormat);
            return EFI_UNSUPPORTED;
        }
    }

    Print(L"[x] Graphics: %ux%u, pitch %u, format %a\n",
        Info->framebuffer_width,
        Info->framebuffer_height,
        Info->framebuffer_pitch,
        (Info->framebuffer_format == PHONON_PIXEL_FORMAT_RGB8) ? "RGB8" : "BGR8"
    );

    return EFI_SUCCESS;
}