/*
	* Shadow/Include/BUS/PCI/PCI.H - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 15:07:24 2026
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
#ifndef __BUS_PCI_PCI_H__
#define __BUS_PCI_PCI_H__

#define PCI_BAR_MEM         (1U << 0)
#define PCI_BAR_IO          (1U << 1)
#define PCI_BAR_PREFETCH    (1U << 2)
#define PCI_BAR_64BIT       (1U << 3)
#define PCI_BAR_VALID       (1U << 4)

/* ECAM */
#define PCI_ECAM_MAX_REGIONS    8
#define PCI_ECAM_BUS_SHIFT      20              /* 1 MiB per bus */
#define PCI_ECAM_BUS_SIZE       (1ULL << 20)
#define PCI_CFG_SPACE_SIZE      4096

/* --- Includes ---*/
#include <BUS/PCI/PCI_IDS.H>

/* --- Typedefs - Structs - Enums ---*/

typedef struct PciBus {
	UINT           Domain;
	UINT           Number;
	struct PciBus *Parent;
} _PciBus, *_PPciBus;

typedef struct PciBAR {
	UINT64          Start;          /* physical address (0 if unset) */
    UINT64          Size;           /* size in bytes (power-of-two) */
    UINT32          Flags;          /* see PCI_BAR_ macros */
} _PciBAR, *_PPciBAR;

typedef struct PciClassCode {
	UINT8 Class;
	UINT8 SubClass;
	UINT8 ProgIF;
	UINT8 Rev;
} _PciClass, *_PPciClass;

typedef struct PciClassName {
	PCCHAR Class;
	PCCHAR SubClass;
	PCCHAR ProgIF;
} _PciClassName, *_PPciClassName;

typedef struct PciDev {
	_PPciBus        Bus;
	UINT            DevFn; /* linux but usefull */
	UINT16          Vendor;
    UINT16          Device;
    _PciClass       Class;
	_PciClassName   ClassName;
    UINT8           HdrType;

	/* Bridge only */
	_PPciBus        Subordinate;
	UINT8           Primary;
	UINT8           Secondary;
	UINT8           SubBus;

    PVOID           DrvData;

	_PciBAR         BAR[6];
	UINT            BARCount; /* Number of BARs is technically always 6, but this is the number of valid bars */
} _PciDev, *_PPciDev;

typedef struct PciBusNode {
    _PciBus             Bus;
    struct PciBusNode  *Next;
} PciBusNode;

typedef struct PciDevNode {
    _PciDev             Dev;
    struct PciDevNode  *Next;
} PciDevNode;

typedef struct PciEcamRegion {
    UINT64  PhysBase;           /* from MCFG */
    UINT    Segment;            /* == Dev->Bus->Domain */
    UINT8   StartBus;
    UINT8   EndBus;
    UINT8   Valid;              /* cleared if self-test fails */
    PVOID   BusMap[256];        /* lazy 1 MiB UC mapping per bus */
} _PciEcamRegion, *_PPciEcamRegion;

typedef struct PciOps {
	SHSTATUS (*Read) (_PPciDev Dev, UINT Offset, SIZE_T Size, PVOID Val);
	SHSTATUS (*Write)(_PPciDev Dev, UINT Offset, SIZE_T Size, UINT64 Val);
} _PciOps, *_PPciOps;

typedef VOID (*_PciWalkFn)   (_PPciDev Dev, PVOID Ctx);
typedef VOID (*_PciCapWalkFn)(_PPciDev Dev, UINT CapPos, PVOID Ctx);

/* --- Globals ---*/

/* --- Prototypes ---*/
#define XAL_EMIT_PROTOTYPES
#include <XAL/xSCAL.H>
#include <BUS/PCI/PCI.XAL>
#undef XAL_EMIT_PROTOTYPES
#undef XAL_METHOD

#endif /* __BUS_PCI_PCI_H__ */