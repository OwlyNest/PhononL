/*
	* Shadow/BUS/PCI/ACPI/MCFG.C - MCFG discovery, ECAM region table
	* Author:   amity
	* Date:     Tue Oct  6 16:06:23 2026
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
#define SDT_HEADER_SIZE     36
#define MCFG_HEADER_SIZE    44              /* SDT header + 8 reserved */
#define MCFG_ENTRY_SIZE     16
#define RSDP_V1_SIZE        20
#define RSDP_V2_SIZE        36
#define ACPI_TABLE_MAX_LEN  0x10000U        /* sanity cap, not a limit */

/* --- Includes ---*/
#include <BUS/PCI/PCI.H>
#include <Lib/Lib.H>
#include <MM/MM.H>

#include <info.h>
/* --- Typedefs - Structs - Enums ---*/

/* Private on-disk layouts. Local names so nothing clashes with ACPICA or other globals */
typedef struct McfgSdtHeader {
    UINT8   Signature[4];
    UINT32  Length;
    UINT8   Revision;
    UINT8   Checksum;
    UINT8   OemId[6];
    UINT8   OemTableId[8];
    UINT32  OemRevision;
    UINT32  CreatorId;
    UINT32  CreatorRevision;
} __attribute__((packed)) McfgSdtHeader;
 
typedef struct McfgRsdp {
    UINT8   Signature[8];
    UINT8   Checksum;
    UINT8   OemId[6];
    UINT8   Revision;
    UINT32  RsdtAddress;
    UINT32  Length;                         /* rev >= 2 only */
    UINT64  XsdtAddress;
    UINT8   ExtChecksum;
    UINT8   Reserved[3];
} __attribute__((packed)) McfgRsdp;
 
typedef struct McfgEntry {
    UINT64  Base;
    UINT16  Segment;
    UINT8   StartBus;
    UINT8   EndBus;
    UINT32  Reserved;
} __attribute__((packed)) McfgEntry;

/* --- Globals ---*/
static _PciEcamRegion Regions[PCI_ECAM_MAX_REGIONS];
static UINT          RegionCount;
extern PhononBootInfo BootInfo;

/* --- Prototypes ---*/

/* --- Functions ---*/

/* Everything below is read through the direct map (WB is fine for RAM). */
static BOOLEAN SigMatch(
	IN PCUCHAR Sig,
	IN PCCHAR Want,
	IN UINT Len
) {
	for (UINT I = 0; I < Len; I++) {
		if (Sig[I] != (UINT8)Want[I]) {
			return FALSE;
		}
	}

	return TRUE;
}

static BOOLEAN ChecksumOk(
	IN PCUCHAR Data,
	IN UINT Len
) {
	UINT8 Sum = 0;

	for (UINT i = 0; i < Len; i++) {
        Sum = (UINT8)(Sum + Data[i]);
    }
 
    return Sum == 0;
}

static UINT64 ReadLe(
    IN PCUCHAR P,
    IN UINT Bytes
) {
    UINT64 Val = 0;
 
    for (UINT i = 0; i < Bytes; i++) {
        Val |= (UINT64)P[i] << (i * 8);
    }
 
    return Val;
}

/* Never hand firmware-supplied addresses to the direct map unchecked. */
static BOOLEAN PhysOk(
    IN PHYS_ADDR_T Phys
) {
    _MM_STATS Stats;
 
    if (Phys == 0) {
        return FALSE;
    }
 
    MmGetPhysicalStats(&Stats);
    return Phys < Stats.HighestPhysical;
}
 
static BOOLEAN SdtSigIs(
    IN PHYS_ADDR_T Phys,
    IN const char *Sig
) {
    if (!PhysOk(Phys)) {
        return FALSE;
    }
 
    return SigMatch(((const McfgSdtHeader *)MmPhysToVirt(Phys))->Signature,Sig, 4);
}

static const McfgSdtHeader *SdtValidate(
    IN PHYS_ADDR_T Phys
) {
    const McfgSdtHeader *Hdr;
 
    if (!PhysOk(Phys)) {
        return NULL;
    }
 
    Hdr = (const McfgSdtHeader *)MmPhysToVirt(Phys);
    if (Hdr->Length < SDT_HEADER_SIZE || Hdr->Length > ACPI_TABLE_MAX_LEN) {
        return NULL;
    }
 
    if (!ChecksumOk((const UINT8 *)Hdr, Hdr->Length)) {
        return NULL;
    }
 
    return Hdr;
}

static PHYS_ADDR_T McfgRsdpPhys(VOID) {
    return BootInfo.rsdp_address;
}

static const McfgSdtHeader *McfgFind(VOID) {
    PHYS_ADDR_T         RsdpPhys = McfgRsdpPhys();
    const McfgRsdp     *Rsdp;
    const McfgSdtHeader *RootHdr;
    const UINT8        *Entries;
    const char         *RootSig;
    PHYS_ADDR_T         Root;
    UINT                EntrySize;
    UINT                Count;
 
    if (!PhysOk(RsdpPhys)) {
        printk("[PCI] MCFG: no RSDP\r\n");
        return NULL;
    }
 
    Rsdp = (const McfgRsdp *)MmPhysToVirt(RsdpPhys);
    if (!SigMatch(Rsdp->Signature, "RSD PTR ", 8) || !ChecksumOk((const UINT8 *)Rsdp, RSDP_V1_SIZE)) {
        printk("[PCI] MCFG: bad RSDP @ 0x%llx\r\n", RsdpPhys);
        return NULL;
    }
 
    if (Rsdp->Revision >= 2
        && Rsdp->Length >= RSDP_V2_SIZE
        && Rsdp->Length <= ACPI_TABLE_MAX_LEN
        && ChecksumOk((const UINT8 *)Rsdp, Rsdp->Length)
        && Rsdp->XsdtAddress != 0) {
        Root      = Rsdp->XsdtAddress;
        RootSig   = "XSDT";
        EntrySize = 8;
    } else {
        Root      = Rsdp->RsdtAddress;
        RootSig   = "RSDT";
        EntrySize = 4;
    }
 
    RootHdr = SdtSigIs(Root, RootSig) ? SdtValidate(Root) : NULL;
    if (RootHdr == NULL) {
        printk("[PCI] MCFG: bad %s @ 0x%llx\r\n", RootSig, Root);
        return NULL;
    }
 
    Entries = (const UINT8 *)RootHdr + SDT_HEADER_SIZE;
    Count   = (RootHdr->Length - SDT_HEADER_SIZE) / EntrySize;
 
    for (UINT i = 0; i < Count; i++) {
        PHYS_ADDR_T Phys = ReadLe(Entries + i * EntrySize, EntrySize);
        const McfgSdtHeader *Hdr;
 
        if (!SdtSigIs(Phys, "MCFG")) {
            continue;
        }
 
        Hdr = SdtValidate(Phys);
        if (Hdr == NULL) {
            printk("[PCI] MCFG: table @ 0x%llx failed validation\r\n", Phys);
            continue;
        }
 
        return Hdr;
    }
 
    return NULL;
}
 
static BOOLEAN RegionOverlaps(
    IN UINT Segment,
    IN UINT Start,
    IN UINT End
) {
    for (UINT i = 0; i < RegionCount; i++) {
        if (Regions[i].Segment == Segment
            && Start <= Regions[i].EndBus
            && End >= Regions[i].StartBus) {
            return TRUE;
        }
    }
 
    return FALSE;
}

SHSTATUS PciMcfgInit(VOID) {
    const McfgSdtHeader *Hdr = McfgFind();
    const UINT8         *Body;
    UINT                 Count;
    UINT8                Oem[7];
 
    RegionCount = 0;
 
    if (Hdr == NULL) {
        printk("[PCI] MCFG: not found, legacy config only\r\n");
        return (SHSTATUS)-1;
    }
 
    if (Hdr->Length < MCFG_HEADER_SIZE) {
        printk("[PCI] MCFG: table too short (%u)\r\n", Hdr->Length);
        return (SHSTATUS)-1;
    }
 
    for (UINT i = 0; i < 6; i++) {
        Oem[i] = Hdr->OemId[i];
    }
    Oem[6] = 0;
 
    Body  = (const UINT8 *)Hdr + MCFG_HEADER_SIZE;
    Count = (Hdr->Length - MCFG_HEADER_SIZE) / MCFG_ENTRY_SIZE;
 
    printk("MCFG: Rev %u OEM '%s' %u Entries\r\n", Hdr->Revision, (const char *)Oem, Count);
 
    for (UINT i = 0; i < Count; i++) {
        const McfgEntry *E = (const McfgEntry *)(Body + i * MCFG_ENTRY_SIZE);
 
        if (E->Base == 0 || (E->Base & (PCI_ECAM_BUS_SIZE - 1)) != 0) {
            printk("MCFG: Entry %u: Bad base 0x%llx, skipped\r\n", i, E->Base);
            continue;
        }
 
        if (E->EndBus < E->StartBus) {
            printk("MCFG: Entry %u: Bus %02x-%02x, skipped\r\n", i, E->StartBus, E->EndBus);
            continue;
        }
 
        if (RegionOverlaps(E->Segment, E->StartBus, E->EndBus)) {
            printk("MCFG: Entry %u: overlaps, skipped\r\n", i);
            continue;
        }
 
        if (RegionCount >= PCI_ECAM_MAX_REGIONS) {
            printk("MCFG: More than %u regions, rest ignored\r\n", PCI_ECAM_MAX_REGIONS);
            break;
        }
 
        Regions[RegionCount].PhysBase = E->Base;
        Regions[RegionCount].Segment  = E->Segment;
        Regions[RegionCount].StartBus = E->StartBus;
        Regions[RegionCount].EndBus   = E->EndBus;
        Regions[RegionCount].Valid    = 1;
        RegionCount++;
 
        printk("ECAM Segment %04x Buses %02x-%02x @ 0x%llx (%u MiB)\r\n", E->Segment, E->StartBus, E->EndBus, E->Base, (UINT)(E->EndBus - E->StartBus + 1));
 
        if (E->StartBus != 0) {
            printk("MCFG: StartBus != 0, base semantics unverified\r\n");
        }
    }
 
    return (RegionCount != 0) ? STATUS_SUCCESS : (SHSTATUS)-1;
}


_PPciEcamRegion PciEcamLookup(
    IN UINT Segment,
    IN UINT Bus
) {
    for (UINT i = 0; i < RegionCount; i++) {
        if (Regions[i].Valid
            && Regions[i].Segment == Segment
            && Bus >= Regions[i].StartBus
            && Bus <= Regions[i].EndBus) {
            return &Regions[i];
        }
    }
 
    return NULL;
}

UINT PciEcamRegionCount(VOID) {
    return RegionCount;
}