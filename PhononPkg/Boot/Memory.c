/*
 * Boot/Memory.c - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:46:25 2026
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
#define MEMORY_EXIT_MAX_ATTEMPTS 3

/* --- Includes ---*/
#include "Uefi/UefiBaseType.h"
#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Memory.h>
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/
EFI_STATUS MemoryExitBootServices(
    IN EFI_HANDLE ImageHandle,
    IN OUT PPhononBootInfo Info
) {
    EFI_STATUS Status;
    UINTN                  Attempt;

    for (Attempt = 0; Attempt < MEMORY_EXIT_MAX_ATTEMPTS; Attempt++) {
        UINTN                  MapSize = 0;
        UINTN                  MapKey;
        UINTN                  DescriptorSize;
        UINT32                 DescriptorVersion;
        EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;

        // Deliberately undersized first call to learn the required size.
        // Safe to log here — this is nowhere near the key we'll eventually
        // commit to below.
        Status = gBS->GetMemoryMap(&MapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
        if (Status != EFI_BUFFER_TOO_SMALL) {
            Print(L"[!] GetMemoryMap(Size) returned unexpected status: %r\n", Status);
            return EFI_LOAD_ERROR;
        }

        Print(L"[x] Memory map needs ~%lu bytes (attempt %lu)...\n",
            (UINT64)MapSize, (UINT64)(Attempt + 1));

        // Everything in this loop — allocating, retrying, logging — is
        // safe: none of it is the call whose MapKey we actually use to exit.
        do {
            MapSize += 2 * DescriptorSize; // pad: AllocatePool itself can grow the map

            Status = gBS->AllocatePool(EfiLoaderData, MapSize, (VOID **)&MemoryMap);
            if (EFI_ERROR(Status)) {
                Print(L"[!] AllocatePool(memory map) failed: %r\n", Status);
                return Status;
            }

            Status = gBS->GetMemoryMap(&MapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
            if (EFI_ERROR(Status)) {
                gBS->FreePool(MemoryMap);
                MemoryMap = NULL;
            }
        } while (Status == EFI_BUFFER_TOO_SMALL);

        if (EFI_ERROR(Status)) {
            Print(L"[!] GetMemoryMap failed: %r\n", Status);
            return Status;
        }

        /* --- Critical section -----------------------------------------
         * NOTHING may run between the GetMemoryMap call above and
         * ExitBootServices below — no Print(), no AllocatePool, no
         * FreePool. Any of those can shift the memory map and invalidate
         * MapKey. This is exactly why there is no logging in this gap,
         * even though it looks like a natural place for a "success!" line.
        */
        Status = gBS->ExitBootServices(ImageHandle, MapKey);
        /* ----------------------------------------------------------------- */

        if (!EFI_ERROR(Status)) {
            // Boot services are gone as of the line above. No gBS, no
            // Print, no Stall from here on — these are just plain memory
            // writes, nothing more.
            Info->memmap_base        = (UINT64)(UINTN)MemoryMap;
            Info->memmap_entry_count = MapSize / DescriptorSize;
            Info->memmap_entry_size  = DescriptorSize;
            return EFI_SUCCESS;
        }

        // ExitBootServices failed — gBS is still guaranteed valid here
        // (the call failed, it didn't half-succeed), so logging and
        // freeing are both safe. Loop around and rebuild the map from
        // scratch rather than recursing: a fixed, bounded number of
        // attempts, no unbounded stack growth if this ever fails
        // repeatedly for a genuine reason instead of a logic bug.
        Print(L"[!] ExitBootServices failed: %r\n", Status);
        gBS->FreePool(MemoryMap);
    }

    Print(L"[!] ExitBootServices did not succeed after %lu attempts\n",
        (UINT64)MEMORY_EXIT_MAX_ATTEMPTS);
    return EFI_ABORTED;
}