/*
	* Shadow/BUS/PCI/Core/DEVICE.C - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 16:13:36 2026
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
#define PCI_MAX_DEVS  256


/* --- Includes ---*/
#include <BUS/PCI/PCI.H>
#include <Lib/Lib.H>
#include <MM/MM.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/

_PPciDev PciDevAlloc(VOID) {
    if (!ExPoolReady()) {
        printk("[PCI] DevAlloc: pool not ready\r\n");
        return NULL;
    }

    _PPciDev Dev = (_PPciDev)ExAllocatePoolZeroed(sizeof(_PciDev));
    if (Dev == NULL) {
        printk("[PCI] DevAlloc: out of memory\r\n");
        return NULL;
    }

    return Dev;
}

VOID PciDevFree(
	IN _PPciDev Dev
) {
    if (Dev == NULL) {
        return;
	}

    ExFreePool(Dev);
}

SHSTATUS PciDevInit(
	IN _PPciDev Dev,
	IN _PPciBus Bus,
	IN UINT DevFn
) {
    if (Dev == NULL || Bus == NULL) {
        return (SHSTATUS)-1;
	}

    Dev->Bus   = Bus;
    Dev->DevFn = DevFn;

    /* read the standard header */
    PciReadConfig(Dev, 0x00, 2, &Dev->Vendor);
    if (Dev->Vendor == 0xFFFF || Dev->Vendor == 0x0000) {
        return (SHSTATUS)-1;
	}

    PciReadConfig(Dev, 0x02, 2, &Dev->Device);
    PciReadConfig(Dev, 0x08, 4, &Dev->Class);
    Dev->Revision = Dev->Class & 0xFF;
    Dev->Class  >>= 8;
    PciReadConfig(Dev, 0x0E, 1, &Dev->HdrType);

    return STATUS_SUCCESS;
}

/* mlem 🦄*/
SHSTATUS PciDevAdd(
	IN _PPciDev Dev
) {
    (VOID)Dev;
    return STATUS_SUCCESS;
}

VOID PciDevRemove(
	IN _PPciDev Dev
) {
	(VOID)Dev;
}

_PPciDev PciDevGet(
	IN _PPciDev Dev
) {
	return Dev;
}

VOID PciDevPut(
	IN _PPciDev Dev
) {
	(VOID)Dev;
}

VOID PciSetDrvData(
	IN _PPciDev Dev,
	IN PVOID Data
) {
	if (Dev) {
		Dev->DrvData = Data;
	}
}

PVOID PciGetDrvData(
	IN _PPciDev Dev
) {
	return Dev ? Dev->DrvData : NULL;
}