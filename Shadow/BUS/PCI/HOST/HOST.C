/*
	* Shadow/BUS/PCI/HOST/HOST.C - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 16:14:37 2026
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
#include "Internal/Types.H"
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

/* --- Includes ---*/
#include <BUS/PCI/PCI.H>
#include <Lib/Lib.H>
#include <Int/IO.H>

#include "../PCI.H"

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/
static UINT32 PciMakeAddress(
	IN _PPciDev Dev,
	IN UINT Offset
) {
	UINT Bus  = Dev->Bus->Number;
	UINT DEV  = PCI_DEV(Dev->DevFn);
	UINT Func = PCI_FUNC(Dev->DevFn);

	return (UINT32)((1U << 31) | ((Bus  & 0xFF) << 16) | ((DEV  & 0x1F) << 11) | ((Func & 0x07) <<  8) | (Offset & 0xFC));
}

static SHSTATUS LegacyRead(
	IN _PPciDev Dev,
	IN UINT Offset,
	IN SIZE_T Size,
	OUT PVOID Val
) {
	UINT32 Addr = PciMakeAddress(Dev, Offset);
	UINT32 Data;

	OutLong(PCI_CONFIG_ADDRESS, Addr);
	Data = InLong(PCI_CONFIG_DATA);

	switch (Size) {
		case 1: {
			*(UINT8 *)Val = (Data >> ((Offset & 3) * 8)) & 0xFF;
			break;
		}

		case 2: {
			*(UINT16 *)Val = (Data >> ((Offset & 2) * 8)) & 0xFFFF;
			break;
		}

		case 4: {
			*(UINT32 *)Val = Data;
			break;
		}

		default: {
			return (SHSTATUS)-1;
		}
	}

	return STATUS_SUCCESS;
}

static SHSTATUS LecacyWrite(
	IN _PPciDev Dev,
	IN UINT Offset,
	IN SIZE_T Size,
	IN UINT64 Val
) {
	UINT32 Addr = PciMakeAddress(Dev, Offset);
    UINT32 Data;
    UINT32 Shift;
    UINT32 Mask;

    OutLong(PCI_CONFIG_ADDRESS, Addr);
    Data = InLong(PCI_CONFIG_DATA);

    switch (Size) {
		case 1: {
			Shift = (Offset & 3) * 8;
			Mask  = 0xFFU << Shift;
			Data  = (Data & ~Mask) | (((UINT32)Val & 0xFF) << Shift);
			break;
		}

		case 2: {
			Shift = (Offset & 2) * 8;
			Mask  = 0xFFFFU << Shift;
			Data  = (Data & ~Mask) | (((UINT32)Val & 0xFFFF) << Shift);
			break;
		}

		case 4: {
			Data = (UINT32)Val;
			break;
		}

		default: {
			return (SHSTATUS)-1;
		}
    }

	OutLong(PCI_CONFIG_ADDRESS, Addr);
	OutLong(PCI_CONFIG_DATA, Data);
	return STATUS_SUCCESS;
}

static _PciOps LegacyOps = {
    .Read  = LegacyRead,
    .Write = LecacyWrite,
};

VOID PciHostInit(VOID) {
    printk("Host: registering legacy CF8/CFC ops\r\n");
    PciConfigOpsRegister(&LegacyOps);
}