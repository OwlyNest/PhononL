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

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

typedef struct PciBus {
	UINT           Domain;
	UINT           Number;
	struct PciBus *Parent;
} _PciBus, *_PPciBus;

typedef struct PciDev {
	_PPciBus        Bus;
	UINT            DevFn; /* linux but usefull */
	UINT16          Vendor;
    UINT16          Device;
    UINT32          Class;
    UINT8           Revision;
    UINT8           HdrType;

	/* Bridge only */
	_PPciBus Subordinate;
	UINT8    Primary;
	UINT8    Secondary;
	UINT8    SubBus;

    PVOID           DrvData;
} _PciDev, *_PPciDev;

typedef struct PciBusNode {
    _PciBus             Bus;
    struct PciBusNode  *Next;
} PciBusNode;

typedef struct PciDevNode {
    _PciDev             Dev;
    struct PciDevNode  *Next;
} PciDevNode;

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