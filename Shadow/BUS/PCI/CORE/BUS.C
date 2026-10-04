/*
	* Shadow/BUS/PCI/Core/BUS.C - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 16:13:30 2026
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
#define PCI_MAX_BUSES      32

/* --- Includes ---*/
#include <BUS/PCI/PCI.H>
#include <Lib/Lib.H>
#include <MM/MM.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
/* --- Prototypes ---*/

/* --- Functions ---*/
_PPciBus PciBusAlloc(VOID) {
    if (!ExPoolReady()) {
        printk("[PCI] BusAlloc: pool not ready\r\n");
        return NULL;
    }

    _PPciBus Bus = (_PPciBus)ExAllocatePoolZeroed(sizeof(_PciBus));
    if (Bus == NULL) {
        printk("[PCI] BusAlloc: out of memory\r\n");
        return NULL;
    }

    return Bus;
}

VOID
PciBusFree(
	IN _PPciBus Bus
) {
    if (Bus == NULL) {
        return;
	}

    ExFreePool(Bus);
}

_PPciBus PciRootBusCreate(
	IN UINT Domain,
	IN UINT BusNr
) {
	_PPciBus Bus = PciBusAlloc();
	if (Bus == NULL) {
		return NULL;
	}

	Bus->Domain = Domain;
	Bus->Number = BusNr;
	Bus->Parent = NULL;

	printk("[PCI] root bus %04x:%02x created\r\n", Domain, BusNr);
    return Bus;
}


/* Emotional support stubs for completion */
INT PciBusAdd(
	IN _PPciBus Bus
) {
    (VOID)Bus;
    return STATUS_SUCCESS;
}

VOID PciBusRemove(
	IN _PPciBus Bus
){
	(VOID)Bus;
}

INT  PciBusAddDevice(
	IN _PPciBus Bus,
	IN _PPciDev Dev
) {
		(VOID)Bus; (VOID)Dev;
		return STATUS_SUCCESS;
	}

VOID PciBusRemoveDevice(
	IN _PPciBus Bus,
	IN _PPciDev Dev) {
	(VOID)Bus;
	(VOID)Dev;
}

VOID PciBusWalk(
	IN _PPciBus Bus,
	IN _PciWalkFn Fn,
	IN PVOID Ctx
) {
	(VOID)Bus;
	(VOID)Fn;
	(VOID)Ctx;
}

_PPciBus PciBusGet(
	IN _PPciBus Bus
) {
	return Bus;
}

VOID PciBusPut(
	IN _PPciBus Bus
) {
	(VOID)Bus;
}