/*
 * Boot/Acpi.c - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:46:53 2026
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
#include "ProcessorBind.h"
#include "Uefi/UefiBaseType.h"
#include "Uefi/UefiSpec.h"
#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

#include <Acpi.h>
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/
EFI_STATUS AcpiFindRsdp(
    IN OUT PPhononBootInfo Info
) {
    UINTN Index;
    VOID *Rdsp20 = NULL;
    VOID *Rdsp10 = NULL;
    VOID *Rdsp;

    for (Index = 0; Index < gST->NumberOfTableEntries; Index++) {
        EFI_CONFIGURATION_TABLE *Entry = &gST->ConfigurationTable[Index];

        if (CompareGuid(&Entry->VendorGuid, &gEfiAcpi20TableGuid)) {
            Rdsp20 = Entry->VendorTable;
        } else if (CompareGuid(&Entry->VendorGuid, &gEfiAcpi10TableGuid)) {
            Rdsp10 = Entry->VendorTable;
        }
    }

    // Prefer ACPI 2.0+ (XSDT, 64-bit table pointers) over 1.0 (RSDT only) when firmware happens to publish both.
    Rdsp = (Rdsp20 != NULL) ? Rdsp20 : Rdsp10;

    if (Rdsp == NULL) {
        Print(L"[!] No ACPI RDSP found in configuration table\n");
        return EFI_NOT_FOUND;
    }

    /* Cheap sanity check: a real RSDP always starts with this 8-byte signature */
    if (AsciiStrnCmp((CONST CHAR8 *)Rdsp, "RSD PTR ", 8) != 0) {
        Print(L"[!] Table at 0x%lx has no RSDP signature\n", (UINT64)(UINTN)Rdsp);
        return EFI_NOT_FOUND;
    }

    Info->rsdp_address = (UINT64)(UINTN)Rdsp;

    Print(L"[x] ACPI RDSP: 0x%lx (%a)\n", Info->rsdp_address, (Rdsp20 != NULL) ? "ACPI 2.0+" : "ACPI 1.0");
    return EFI_SUCCESS;
}