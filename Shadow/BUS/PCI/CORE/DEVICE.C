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
        printk("DevAlloc: pool not ready\r\n");
        return NULL;
    }

    _PPciDev Dev = (_PPciDev)ExAllocatePoolZeroed(sizeof(_PciDev));
    if (Dev == NULL) {
        printk("DevAlloc: out of memory\r\n");
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

    /*
		* Read the standard header:

		* | Register | Offset | Byte 4      | Byte 3      | Byte 2        | Byte 1               |
		* | 0x0      | 0x0    | Device ID                 | Vendor ID                            |
		* | 0x1      | 0x4    | Status                    | Command                              |
		* | 0x2      | 0x8    | Class code  | Subclass    | Prog IF       | Revision ID          |
		* | 0x3      | 0xC    | BIST        | Header type | Latency Timer | Cache Line Size      |
		* | 0x4      | 0x10   | Base address #0 (BAR0)                                           |
		* | 0x5      | 0x14   | Base address #1 (BAR1)                                           |
		* | 0x6      | 0x18   | Base address #2 (BAR2)                                           |
		* | 0x7      | 0x1C   | Base address #3 (BAR3)                                           |
		* | 0x8      | 0x20   | Base address #4 (BAR4)                                           |
		* | 0x9      | 0x24   | Base address #5 (BAR5)                                           |
		* | 0xA      | 0x28   | Cardbus CIS Pointer                                              |
		* | 0xB      | 0x2C   | Subsystem ID              | Subsystem Vendor ID                  |
		* | 0xC      | 0x30   | Expansion ROM base address                                       |
		* | 0xD      | 0x34   | Reserved                                  | Capabilities Pointer |
		* | 0xE      | 0x38   | Reserved                                                         |
		* | 0xF      | 0x3C   | Max latency | Min Grant   | Interrupt PIN | Interrupt Line       |
	*/
    PciReadConfig(Dev, 0x00, 2, &Dev->Vendor);
    if (Dev->Vendor == 0xFFFF || Dev->Vendor == 0x0000) {
        return (SHSTATUS)-1;
	}

    PciReadConfig(Dev, 0x02, 2, &Dev->Device);
    UINT32 ClassReg;

	PciReadConfig(Dev, 0x08, 4, &ClassReg);

	Dev->Class.Class    = (UINT8)((ClassReg >> 24) & 0xFF);
	Dev->Class.SubClass = (UINT8)((ClassReg >> 16) & 0xFF);
	Dev->Class.ProgIF   = (UINT8)((ClassReg >>  8) & 0xFF);
	Dev->Class.Rev      = (UINT8)((ClassReg >>  0) & 0xFF);
	PciDecodeClass(Dev);
    PciReadConfig(Dev, 0x0E, 1, &Dev->HdrType);

    return STATUS_SUCCESS;
}

/* mlem 🦄*/
/*
	* To be extremely clear:
	* Present in the next three tables does NOT gaurantee suppoprt
	* The next three tables decode every PCI Class code, Subclass code and Program InterFace code I could find in multiple sources:
	* https://admin.pci-ids.ucw.cz/read/PD/ 
	* Credits to Albert Pool and Martin Mares (https://mj.ucw.cz/)
	* This repository also contains a full list of vendors and Devices.
	* But the vendor table contains a cool 2823 entries. EACH with a variable set of devices.
	* The Intel set alone contains 5761 entries.
	* So in conclusion, Do that yourself
	* https://wiki.osdev.org/PCI#Class_Codes
*/
static PCCHAR PciDecodeClassCode(
	IN _PciClass Class
) {
	switch (Class.Class) {
		case 0x00:
			return "Unclassified";
		case 0x01:
			return "Mass Storage Controller";
		case 0x02:
			return "Network Controller";
		case 0x03:
			return "Display Controller";
		case 0x04:
			return "Multimedia Controller";
		case 0x05:
			return "Memory Controller";
		case 0x06:
			return "Bridge";
		case 0x07:
			return "Communication Controller";
		case 0x08:
			return "Generic System Peripheral";
		case 0x09:
			return "Input Device Controller";
		case 0x0A:
			return "Docking Station";
		case 0x0B:
			return "Processor";
		case 0x0C:
			return "Serial Bus Controller";
		case 0x0D:
			return "Wireless Controller ";
		case 0x0E:
			return "Intelligent Controller";
		case 0x0F:
			return "Sattelite Communications Controller ";
		case 0x10:
			return "Encryption Controller";
		case 0x11:
			return "Signal Processing Controller";
		case 0x12:
			return "Processing Accelerators";
		case 0x13:
			return "Non-Essential Instrumentation";
		case 0x40:
			return "Co-Processor";
		case 0xFF:
			return "Unassigned";
		default:
			return "Unknown";
	}
}

static PCCHAR PciDecodeSubCode(
	IN _PciClass Class
) {
	switch (Class.Class) {
		case 0x00:
			switch(Class.SubClass) {
				case 0x00:
					return "Non-VGA Unclassified Device";
				case 0x01:
					return "VGA Compatible Unclassified Device";
				case 0x05:
					return "Image Co-Processor";
				default:
					return "Unknown";
			}
		case 0x01:
			switch(Class.SubClass) {
				case 0x00:
					return "SCSI Storage Controller";
				case 0x01:
					return "IDE Interface";
				case 0x02:
					return "Floppy Disk Controller";
				case 0x03:
					return "IPI Bus Controller";
				case 0x04:
					return "RAID Bus Controller";
				case 0x05:
					return "ATA Controller";
				case 0x06:
					return "SATA Controller";
				case 0x07:
					return "Serial Attached SCSI Controller";
				case 0x08:
					return "NVM Controller";
				case 0x09:
					return "Universal Flash Storage Controller";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x02:
			switch(Class.SubClass) {
				case 0x00:
					return "Ethernet Controller";
				case 0x01:
					return "Token Ring Controller";
				case 0x02:
					return "FDDI Controller";
				case 0x03:
					return "ATM Controller";
				case 0x04:
					return "ISDN Controller";
				case 0x05:
					return "WorldFip Controller";
				case 0x06:
					return "PICMG 2.14 Multi Computing Controller";
				case 0x07:
					return "InfiniBand Controller";
				case 0x08:
					return "Fabric Controller";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x03:
			switch(Class.SubClass) {
				case 0x00:
					return "VGA Compatible Controller";
				case 0x01:
					return "XGA Controller";
				case 0x02:
					return "3D Controller";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x04:
			switch(Class.SubClass) {
				case 0x00:
					return "Multimedia Video Controller";
				case 0x01:
					return "Multimedia Audio Controller";
				case 0x02:
					return "Computer Telephony Device";
				case 0x03:
					return "Audio Device";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x05:
			switch(Class.SubClass) {
				case 0x00:
					return "RAM Controller";
				case 0x01:
					return "FLASH Controller";
				case 0x02:
					return "CXL";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x06:
			switch(Class.SubClass) {
				case 0x00:
					return "Host Bridge";
				case 0x01:
					return "ISA Bridge";
				case 0x02:
					return "EISA Bridge";
				case 0x03:
					return "MCA Bridge";
				case 0x04:
					return "PCI to PCI Bridge";
				case 0x05:
					return "PCMCIA Bridge";
				case 0x06:
					return "NuBus Bridge";
				case 0x07:
					return "CardBus Bridge";
				case 0x08:
					return "RACEway Bridge";
				case 0x09:
					return "Semi-transparent PCI to PCI Bridge";
				case 0x0A:
					return "InfiniBand to PCI Host Bridge";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x07:
			switch(Class.SubClass) {
				case 0x00:
					return "Serial Controller";
				case 0x01:
					return "Parallel Controller";
				case 0x02:
					return "Multiport Serial Controller";
				case 0x03:
					return "Modem";
				case 0x04:
					return "IEEE 488.1/2 (GPIB) Controller";
				case 0x05:
					return "Smart Card Controller";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x08:
			switch(Class.SubClass) {
				case 0x00:
					return "PIC";
				case 0x01:
					return "DMA Controller";
				case 0x02:
					return "Timer";
				case 0x03:
					return "RTC Controller";
				case 0x04:
					return "PCI Hot-Plug Controller";
				case 0x05:
					return "SD Host Controller";
				case 0x06:
					return "IOMMU";
				case 0x07:
					return "Root Complex Event Collector";
				case 0x80:
					return "Other";
				case 0x99:
				default:
					return "Unknown";
			}
		case 0x09:
			switch(Class.SubClass) {
				case 0x00:
					return "Keyboard Controller";
				case 0x01:
					return "Digitizer Pen";
				case 0x02:
					return "Mouse Controller";
				case 0x03:
					return "Scanner Controller";
				case 0x04:
					return "Gameport Controller";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x0A:
			switch(Class.SubClass) {
				case 0x00:
					return "Generic";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x0B:
			switch(Class.SubClass) {
				case 0x00:
					return "386";
				case 0x01:
					return "486	";
				case 0x02:
					return "Pentium";
				case 0x03:
					return "Pentium Pro";
				case 0x10:
					return "Alpha";
				case 0x20:
					return "PowerPC";
				case 0x30:
					return "MIPS";
				case 0x40:
					return "Co-Processor";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x0C:
			switch(Class.SubClass) {
				case 0x00:
					return "FireWire (IEEE 1394) Controller";
				case 0x01:
					return "ACCESS Bus Controller";
				case 0x02:
					return "SSA";
				case 0x03:
					return "USB Controller";
				case 0x04:
					return "Fibre Channnel";
				case 0x05:
					return "SMBus Controller";
				case 0x06:
					return "InfiniBand Controller";
				case 0x07:
					return "IPMI Interface";
				case 0x08:
					return "SERCOS Interface (IEC 61491)";
				case 0x09:
					return "CANbus Controller";
				case 0x0A:
					return "MIPI I3C";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x0D:
			switch(Class.SubClass) {
				case 0x00:
					return "iRDA Compatible Controller";
				case 0x01:
					return "Consumer IR Controller";
				case 0x10:
					return "RF Controller";
				case 0x11:
					return "Bluetooth Controller";
				case 0x12:
					return "Broadband Controller";
				case 0x20:
					return "802.11a 5 GHz Controller";
				case 0x21:
					return "802.11b 2.4 GHz Controller";
				case 0x40:
					return "Cellular Controller/Modem";
				case 0x41:
					return "Cellular Controller/Modem Plus Ethernet (802.11)";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x0E:
			switch(Class.SubClass) {
				case 0x00:
					return "I2O";
				default:
					return "Unknown";
			}
		case 0x0F:
			switch(Class.SubClass) {
				case 0x01:
					return "Satellite TV Controller";
				case 0x02:
					return "Satellite Audio Controller";
				case 0x03:
					return "Satellite Void Controller";
				case 0x04:
					return "Satellite Data Controller";
				default:
					return "Unknown";
			}
		case 0x10:
			switch(Class.SubClass) {
				case 0x00:
					return "Network and Computing Encryption Device";
				case 0x10:
					return "Entertainment Encryption Device";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x11:
			switch(Class.SubClass) {
				case 0x00:
					return "DPIO module";
				case 0x01:
					return "Performance Counters";
				case 0x10:
					return "Communication Synchronizer";
				case 0x20:
					return "Signal Processing Management";
				case 0x80:
					return "Other";
				default:
					return "Unknown";
			}
		case 0x12:
			switch(Class.SubClass) {
				case 0x00:
					return "Others";
				case 0x01:
					return "SNIA Smart Data Accelerator Interface (SDXI) controller";
				default:
					return "Unknown";
			}
		case 0x13:
			return "Non-Essential Instrumentation";
		case 0x40:
			return "Co-Processor";
		case 0xFF:
			return "Unassigned";
		default:
			return "Unknown";
	}
}

static PCCHAR PciDecodeProgIF(
	IN _PciClass Class
) {
	switch (Class.Class) {
		case 0x01:
			switch(Class.SubClass) {
				case 0x01:
					switch (Class.ProgIF) {
						case 0x00:
							return "ISA Compatibility mode-only controller";
						case 0x05:
							return "PCI native mode-only controller";
						case 0x0A:
							return "ISA Compatibility mode controller, supports both channels switched to PCI native mode";
						case 0x0F:
							return "PCI native mode controller, supports both channels switched to ISA compatibility mode";
						case 0x80:
							return "ISA Compatibility mode-only controller, supports bus mastering";
						case 0x85:
							return "PCI native mode-only controller, supports bus mastering";
						case 0x8A:
							return "ISA Compatibility mode controller, supports both channels switched to PCI native mode, supports bus mastering";
						case 0x8F:
							return "PCI native mode controller, supports both channels switched to ISA compatibility mode, supports bus mastering";
						default:
							return "-";
					}
				case 0x05:
					switch (Class.ProgIF) {
						case 0x20:
							return "Single DMA";
						case 0x30:
							return "Chained DMA";
						default:
							return "-";
					}
				case 0x06:
					switch (Class.ProgIF) {
						case 0x00:
							return "Vendor Specific Interface";
						case 0x01:
							return "AHCI 1.0";
						case 0x02:
							return "Serial Storage Bus";
						default:
							return "-";
					}
				case 0x07:
					switch (Class.ProgIF) {
						case 0x00:
							return "SAS";
						case 0x01:
							return "Serial Storage Bus";
						default:
							return "-";
					}
				case 0x08:
					switch (Class.ProgIF) {
						case 0x01:
							return "NVMHCI";
						case 0x02:
							return "NVM Express";
						default:
							return "-";
					}
				case 0x09:
					switch (Class.ProgIF) {
						case 0x00:
							return "Vendor Specific";
						case 0x01:
							return "UFSHCI";
						default:
							return "-";
					}
				default:
					return "-";
			}
		case 0x03:
			switch(Class.SubClass) {
				case 0x00:
					switch (Class.ProgIF) {
						case 0x00:
							return "VGA Controller";
						case 0x01:
							return "8514 Compatible Controller";
						default:
							return "-";
					}
				default:
					return "-";
			}
		case 0x06:
			switch(Class.SubClass) {
				case 0x04:
					switch (Class.ProgIF) {
						case 0x00:
							return "Normal Decode";
						case 0x01:
							return "Subtractive Decode";
						default:
							return "-";
					}
				case 0x08:
					switch (Class.ProgIF) {
						case 0x00:
							return "Transparent Mode";
						case 0x01:	
							return "Endpoint Mode";
						default:
							return "-";
					}
				case 0x09:
					switch (Class.ProgIF) {
						case 0x40:
							return "Semi-Transparent, Primary Bus Towards Host CPU";
						case 0x80:
							return "Semi-Transparent, Secondary Bus Towards Host CPU";
						default:
							return "-";
					}
				default:
					return "-";
			}
		case 0x07:
			switch(Class.SubClass) {
				case 0x00:
					switch (Class.ProgIF) {
						case 0x00:
							return "8250-Compatible (Generic XT)";
						case 0x01:
							return "16450-Compatible";
						case 0x02:
							return "16550-Compatible";
						case 0x03:
							return "16650-Compatible";
						case 0x04:
							return "16750-Compatible";
						case 0x05:
							return "16850-Compatible";
						case 0x06:
							return "16950-Compatible";
						default:
							return "-";
					}
				case 0x01:
					switch (Class.ProgIF) {
						case 0x00:
							return "Standard Parallel Port";
						case 0x01:
							return "Bi-Directional Parallel Port";
						case 0x02:
							return "ECP 1.X Compliant Parallel Port";
						case 0x03:
							return "IEEE 1284 Controller";
						case 0xFE:
							return "IEEE 1284 Target Device";
						default:
							return "-";
					}
				case 0x03:
					switch (Class.ProgIF) {
						case 0x00:
							return "Generic Modem";
						case 0x01:
							return "Hayes 16450-Compatible Interface";
						case 0x02:
							return "Hayes 16550-Compatible Interface";
						case 0x03:
							return "Hayes 16650-Compatible Interface";
						case 0x04:
							return "Hayes 16750-Compatible Interface";
						default:
							return "-";
					}
				default:
					return "-";
			}
		case 0x08:
			switch(Class.SubClass) {
				case 0x00:
					switch (Class.ProgIF) {
						case 0x00:
							return "8259 Compatible (Generic)";
						case 0x01:
							return "ISA Compatible";
						case 0x02:
							return "EISA Compatible";
						case 0x10:
							return "I/O APIC Interrupt Controller";
						case 0x20:
							return "I/O(x) APIC Interrupt Controller";
						default:
							return "-";
					}
				case 0x01:
					switch (Class.ProgIF) {
						case 0x00:
							return "8237 Compatible (Generic)";
						case 0x01:
							return "ISA Compatible";
						case 0x02:
							return "EISA Compatible";
						default:
							return "-";
					}
				case 0x02:
					switch (Class.ProgIF) {
						case 0x00:
							return "8254 Compatible (Generic)";
						case 0x01:
							return "ISA Compatible";
						case 0x02:
							return "EISA Compatible";
						case 0x03:
							return "HPET";
						default:
							return "-";
					}
				case 0x03:
					switch (Class.ProgIF) {
						case 0x00:
							return "Generic";
						case 0x01:
							return "ISA Compatible";
						default:
							return "-";
					}
				default:
					return "-";
			}
		case 0x09:
			switch(Class.SubClass) {
				case 0x04:
					switch (Class.ProgIF) {
						case 0x00:
							return "Generic";
						case 0x10:
							return "Extended";
						default:
							return "-";
					}
				default:
					return "-";
			}
		case 0x0C:
			switch(Class.SubClass) {
				case 0x00:
					switch (Class.ProgIF) {
						case 0x00:
							return "Generic";
						case 0x10:
							return "OHCI";
						default:
							return "-";
					}
				case 0x03:
					switch (Class.ProgIF) {
						case 0x00:
							return "UHCI Controller";
						case 0x10:
							return "OHCI Controller";
						case 0x20:
							return "EHCI (USB2) Controller";
						case 0x30:
							return "XHCI (USB3) Controller";
						case 0x80:
							return "Unspecified";
						case 0xFE:
							return "USB Device";
						default:
							return "-";
					}
				case 0x07:
					switch (Class.ProgIF) {
						case 0x00:
							return "SMIC";
						case 0x01:
							return "Keyboard Controller Style";
						case 0x02:
							return "Block Transfer";
						default:
							return "-";
					}
				default:
					return "-";
			}
		default:
			return "-";
	}
}

VOID PciDecodeClass(
	IN _PPciDev Dev
) {
	Dev->ClassName.Class    = PciDecodeClassCode(Dev->Class);
	Dev->ClassName.SubClass = PciDecodeSubCode(Dev->Class);
	Dev->ClassName.ProgIF   = PciDecodeProgIF(Dev->Class);
}


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