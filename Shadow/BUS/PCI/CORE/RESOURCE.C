/*
	* Shadow/BUS/PCI/Core/RESOURCE.C - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 16:13:55 2026
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

#include "../PCI.H"
/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static UINT64 IoNext   = PCI_IO_WINDOW_START;
static UINT64 Mem32Next = PCI_MEM32_START;
static UINT64 Mem64Next = PCI_MEM64_START;
static INT WindowsKnown = 0;

/* --- Prototypes ---*/

/* --- Functions ---*/

static UINT64 AlignUp(UINT64 Value, UINT64 Align) {
	return (Value + Align - 1) & ~(Align - 1);
}

static UINT64 PciBARSize(
	IN UINT32 Raw
) {
	if (Raw & 1) { /* I/O BAR */
		Raw &= ~0x3U;
		if (Raw == 0) {
			return 0;
		}

		return (UINT64)(~Raw + 1) & 0xFFFF;

	} else { /* Memory BAR */
		Raw &= ~0xFU;
		if (Raw == 0) {
			return 0;
		}

		return (UINT64)(~Raw + 1);
	}
}

SHSTATUS PciReadBases(
	IN _PPciDev Dev
) {
	UINT16 Cmd = 0;
	UINT MaxBARs;
	UINT i;

	if (Dev == NULL) {
		return (SHSTATUS)-1;
	}

	/* A bridge only has two BAR slots */
	MaxBARs = (PCI_IS_BRIDGE(Dev->HdrType)) ? 2 : 6;
	Dev->BARCount = 0;

	/* Decode off while BARs hold 0xFFFFFFFF. No printk until restored! */
	PciReadConfig(Dev, PCI_REG_COMMAND, 2, &Cmd);
	PciWriteConfig(Dev, PCI_REG_COMMAND, 2,
	               (UINT16)(Cmd & ~(PCI_CMD_IO | PCI_CMD_MEM)));

	for (i = 0; i < MaxBARs; ) {
		UINT   Offset   = PCI_REG_BAR0 + i * 4;
		UINT32 OrigLow  = 0;
		UINT32 OrigHigh = 0;
		UINT32 Low      = 0;
		UINT32 High     = 0;
		UINT32 Flags;
		UINT64 Size;
		UINT64 Start;
		UINT   Slots    = 1;

		PciReadConfig(Dev, Offset, 4, &OrigLow);
		PciWriteConfig(Dev, Offset, 4, 0xFFFFFFFF);
		PciReadConfig(Dev, Offset, 4, &Low);
		PciWriteConfig(Dev, Offset, 4, OrigLow);

		if (Low == 0 || Low == 0xFFFFFFFF) {
			i++;
			continue;
		}

		if (Low & 1) {                      /* I/O space */
			Flags = PCI_BAR_IO | PCI_BAR_VALID;
			Size  = PciBARSize(Low);
			Start = OrigLow & ~0x3U;        /* firmware's assignment */
		} else {                            /* Memory space */
			Flags = PCI_BAR_MEM | PCI_BAR_VALID;
			Start = OrigLow & ~0xFU;

			if (Low & 0x08) {
				Flags |= PCI_BAR_PREFETCH;
			}

			if ((Low & 0x06) == 0x04) {     /* 64-bit, uses next slot */
				if (i + 1 >= MaxBARs) {
					break;
				}

				PciReadConfig(Dev, Offset + 4, 4, &OrigHigh);
				PciWriteConfig(Dev, Offset + 4, 4, 0xFFFFFFFF);
				PciReadConfig(Dev, Offset + 4, 4, &High);
				PciWriteConfig(Dev, Offset + 4, 4, OrigHigh);

				Flags |= PCI_BAR_64BIT;
				Start |= (UINT64)OrigHigh << 32;
				Size   = ~(((UINT64)High << 32) | (Low & ~0xFU)) + 1;
				Slots  = 2;
			} else {
				Size = PciBARSize(Low);
			}
		}

		Dev->BAR[i].Start = Start;          /* 0 means unassigned */
		Dev->BAR[i].Size  = Size;
		Dev->BAR[i].Flags = Flags;

		i += Slots;
		Dev->BARCount++;
	}

	PciWriteConfig(Dev, PCI_REG_COMMAND, 2, Cmd);

	/* Aligned BAR lines – sit under the device description */
	for (i = 0; i < 6; i++) {
		_PPciBAR BAR = &Dev->BAR[i];

		if (!(BAR->Flags & PCI_BAR_VALID)) {
			continue;
		}

		PCCHAR Type;
		if (BAR->Flags & PCI_BAR_IO) {
			Type = "I/O  ";
		} else if (BAR->Flags & PCI_BAR_64BIT) {
			Type = "mem64";
		} else {
			Type = "mem32";
		}

		printk("           BAR%u  %s  0x%012llx  size 0x%08llx\r\n",
		       i, Type, BAR->Start, BAR->Size);
	}

	return STATUS_SUCCESS;
}


/* stubby stubby stub stubs */
SHSTATUS PciAssignResource(
	IN _PPciDev Dev,
	IN UINT BARIdx
) {
	if (Dev == NULL || BARIdx >= 6) {
		return (SHSTATUS)-1;
	}

	_PPciBAR BAR = &Dev->BAR[BARIdx];
	if (!(BAR->Flags & PCI_BAR_VALID) || BAR->Size == 0) {
		return STATUS_SUCCESS;
	}

	if (!WindowsKnown) {
		printk("           BAR%u  unassigned (no resource windows)\r\n",
		       BARIdx);
		return (SHSTATUS)-1;
	}

	UINT64 Addr = 0;
	UINT64 Align = BAR->Size;

	if (BAR->Flags & PCI_BAR_IO) {
		Addr = AlignUp(IoNext, Align);
		if (Addr + BAR->Size > PCI_IO_WINDOW_END) {
			printk("I/O window exhausted for BAR%d\r\n", BARIdx);
			return (SHSTATUS)-1;
		}
        
		IoNext = Addr + BAR->Size;
	}
	else if (BAR->Flags & PCI_BAR_64BIT) {
		Addr = AlignUp(Mem64Next, Align);
		if (Addr + BAR->Size > PCI_MEM64_END) {
			printk("64-bit mem window exhausted for BAR%d\r\n", BARIdx);
			return (SHSTATUS)-1;
		}
		Mem64Next = Addr + BAR->Size;
	}
	else {  /* 32-bit memory */
		Addr = AlignUp(Mem32Next, Align);
		if (Addr + BAR->Size > PCI_MEM32_END) {
			printk("32-bit mem window exhausted for BAR%d\r\n", BARIdx);
			return (SHSTATUS)-1;
		}

		Mem32Next = Addr + BAR->Size;
	}

	BAR->Start = Addr;

	UINT Offset = 0x10 + BARIdx * 4;
	UINT32 Low = 0;

	PciReadConfig(Dev, Offset, 4, &Low);

	if (BAR->Flags & PCI_BAR_IO) {
		Low = (UINT32)(Addr & 0xFFFFFFFC) | (Low & 0x3);
		PciWriteConfig(Dev, Offset, 4, Low);
	}
	else if (BAR->Flags & PCI_BAR_64BIT) {
		Low = (UINT32)(Addr & 0xFFFFFFF0) | (Low & 0xF);
		PciWriteConfig(Dev, Offset, 4, Low);
		PciWriteConfig(Dev, Offset + 4, 4, (UINT32)(Addr >> 32));
	}
	else {
		Low = (UINT32)(Addr & 0xFFFFFFF0) | (Low & 0xF);
		PciWriteConfig(Dev, Offset, 4, Low);
	}

	printk("           BAR%u  assigned  0x%012llx  size 0x%08llx\r\n", BARIdx, BAR->Start, BAR->Size);

	return STATUS_SUCCESS;
}

SHSTATUS PciAssignUnassignedResources(
	IN _PPciBus Bus
) {
	(VOID)Bus;

	return STATUS_SUCCESS;
}

SHSTATUS PciUpdateResource(
	IN _PPciDev Dev,
	IN UINT BARIdx
) {
	(VOID)Dev;
	(VOID)BARIdx;

	return STATUS_SUCCESS;
}