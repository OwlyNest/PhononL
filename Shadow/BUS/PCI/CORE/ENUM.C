/*
	* Shadow/BUS/PCI/Core/ENUM.C - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 16:13:44 2026
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
#include <BUS/PCI/PCI.H>
#include <Lib/Lib.H>
#include <DRV/PS2/PS2.H>

#include "../PCI.H"

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static UINT NextBusNumber = 1; /* root is 0 */

/* --- Prototypes ---*/

/* --- Functions ---*/

SHSTATUS PciScanFunction(
	IN _PPciBus Bus,
	IN UINT DevFn
) {
	_PPciDev Dev = PciDevAlloc();
	if (Dev == NULL) {
		return (SHSTATUS)-1;
	}

	INT Rc = PciDevInit(Dev, Bus, DevFn);
	if (Rc != STATUS_SUCCESS) {
		PciDevFree(Dev);
		return Rc;
	}

	printk("[PCI] %04x:%02x:%02x.%x  %04x:%04x  Class %06x  Hdr %02x\r\n", Bus->Domain, Bus->Number, PCI_DEV(DevFn), PCI_FUNC(DevFn), Dev->Vendor, Dev->Device, Dev->Class, Dev->HdrType);

	PciBusAddDevice(Bus, Dev);
	PciDevAdd(Dev);

	if ((Dev->HdrType & 0x7F) == 0x01) {
		PciScanBridge(Dev);
	}

	if (PCI_FUNC(DevFn) == 0 && (Dev->HdrType & 0x80)) {
        for (UINT F = 1; F < 8; F++) {
            PciScanFunction(Bus, PCI_DEVFN(PCI_DEV(DevFn), F));
        }
    }

	return STATUS_SUCCESS;
}

SHSTATUS PciScanSlot(
	IN _PPciBus Bus,
	IN UINT Slot
) {
	return PciScanFunction(Bus, PCI_DEVFN(Slot, 0));
}

SHSTATUS PciScanBus(
	IN _PPciBus Bus
) {
    printk("[PCI] scanning bus %04x:%02x\r\n", Bus->Domain, Bus->Number);
    Kbr();

    for (UINT Slot = 0; Slot < 32; Slot++) {
        PciScanSlot(Bus, Slot);
    }
    return STATUS_SUCCESS;
}

SHSTATUS PciEnumerate(VOID) {
	_PPciBus Root = PciRootBusCreate(0, 0);
	if (Root == NULL) {
		return (SHSTATUS)-1;
	}

	PciBusAdd(Root);
    return PciScanBus(Root);
}

/* I stubbed my toe */
SHSTATUS PciScanDevice(
	IN _PPciBus Bus,
	IN UINT Slot,
	IN UINT Func
) {
    return PciScanFunction(Bus, PCI_DEVFN(Slot, Func));
}

SHSTATUS PciScanBridge(
	IN _PPciDev Bridge
) {
    if ((Bridge->HdrType & 0x7F) != 0x01) {
        return (SHSTATUS)-1;
	}

    _PPciBus SecBus = PciBusAlloc();
    if (SecBus == NULL) {
        return (SHSTATUS)-1;
	}

    SecBus->Domain = Bridge->Bus->Domain;
    SecBus->Parent = Bridge->Bus;
    SecBus->Number = NextBusNumber++;

    Bridge->Subordinate = SecBus;
    Bridge->Primary     = (UINT8)Bridge->Bus->Number;
    Bridge->Secondary   = (UINT8)SecBus->Number;

    Bridge->SubBus = Bridge->Secondary;

    /* program the bridge with a temporary window so devices behind it
       can answer config cycles */
    PciWriteConfig(Bridge, 0x18, 1, Bridge->Primary);
    PciWriteConfig(Bridge, 0x19, 1, Bridge->Secondary);
    PciWriteConfig(Bridge, 0x1A, 1, Bridge->SubBus);   /* will update */

    printk("[PCI] bridge %04x:%02x:%02x.%x  -> sec %02x\r\n", Bridge->Bus->Domain, Bridge->Bus->Number, PCI_DEV(Bridge->DevFn), PCI_FUNC(Bridge->DevFn), SecBus->Number);
    Kbr();

    INT Rc = PciScanBus(SecBus);

    Bridge->SubBus = (UINT8)(NextBusNumber - 1);

    PciWriteConfig(Bridge, 0x1A, 1, Bridge->SubBus);

    printk("[PCI] bridge %04x:%02x:%02x.%x  final window %02x-%02x\r\n", Bridge->Bus->Domain, Bridge->Bus->Number, PCI_DEV(Bridge->DevFn), PCI_FUNC(Bridge->DevFn), Bridge->Secondary, Bridge->SubBus);

    return Rc;
}

VOID PciAssignBusNumbers(
	IN _PPciBus Bus,
	IN UINT *NextBus
) {
	(VOID)Bus;
	(VOID)NextBus;
}