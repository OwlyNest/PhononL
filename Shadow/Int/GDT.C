/*
	* Shadow/Int/GDT.C - Global Descriptor Table
	* Author:   amity
	* Date:     Wed Sep 23 22:25:41 2026
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

// Each define here is for a specific flag in the descriptor.
// Refer to the intel documentation for a description of what each one does.
#define SEG_DESCTYPE(x)  ((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define SEG_PRES(x)      ((x) << 0x07) // Present
#define SEG_SAVL(x)      ((x) << 0x0C) // Available for system use
#define SEG_LONG(x)      ((x) << 0x0D) // Long mode
#define SEG_SIZE(x)      ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define SEG_GRAN(x)      ((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define SEG_PRIV(x)     (((x) &  0x03) << 0x05)   // Set privilege level (0 - 3)
 
#define SEG_DATA_RD        0x00 // Read-Only
#define SEG_DATA_RDA       0x01 // Read-Only, accessed
#define SEG_DATA_RDWR      0x02 // Read/Write
#define SEG_DATA_RDWRA     0x03 // Read/Write, accessed
#define SEG_DATA_RDEXPD    0x04 // Read-Only, expand-down
#define SEG_DATA_RDEXPDA   0x05 // Read-Only, expand-down, accessed
#define SEG_DATA_RDWREXPD  0x06 // Read/Write, expand-down
#define SEG_DATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define SEG_CODE_EX        0x08 // Execute-Only
#define SEG_CODE_EXA       0x09 // Execute-Only, accessed
#define SEG_CODE_EXRD      0x0A // Execute/Read
#define SEG_CODE_EXRDA     0x0B // Execute/Read, accessed
#define SEG_CODE_EXC       0x0C // Execute-Only, conforming
#define SEG_CODE_EXCA      0x0D // Execute-Only, conforming, accessed
#define SEG_CODE_EXRDC     0x0E // Execute/Read, conforming
#define SEG_CODE_EXRDCA    0x0F // Execute/Read, conforming, accessed
 
#define GDT_CODE_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(1)     | SEG_SIZE(0) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL0 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(0)     | SEG_DATA_RDWR
 
#define GDT_CODE_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_CODE_EXRD
 
#define GDT_DATA_PL3 SEG_DESCTYPE(1) | SEG_PRES(1) | SEG_SAVL(0) | \
                     SEG_LONG(0)     | SEG_SIZE(1) | SEG_GRAN(1) | \
                     SEG_PRIV(3)     | SEG_DATA_RDWR

/* --- Includes ---*/
#include <XAL/XScope.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static UINT64 Gdt[3];

struct GdtPtr {
	UINT16 Limit;
	UINT64 Base;
} __attribute__((packed));

static struct GdtPtr gp;

/* --- Prototypes ---*/

/* --- Functions ---*/

UINT64 CreateDescriptor(
	IN UINT32 Base,
	IN UINT32 Limit,
	IN UINT16 Flag
) {
    UINT64 Descriptor;
 
    // Create the high 32 bit segment
    Descriptor  =  Limit       & 0x000F0000;         // set limit bits 19:16
    Descriptor |= (Flag <<  8) & 0x00F0FF00;         // set type, p, dpl, s, g, d/b, l and avl fields
    Descriptor |= (Base >> 16) & 0x000000FF;         // set base bits 23:16
    Descriptor |=  Base        & 0xFF000000;         // set base bits 31:24
 
    // Shift by 32 to allow for low part of segment
    Descriptor <<= 32;
 
    // Create the low 32 bit segment
    Descriptor |= Base  << 16;                       // set base bits 15:0
    Descriptor |= Limit  & 0x0000FFFF;               // set limit bits 15:0

	return Descriptor;
}

VOID GdtInit(VOID) {
	Gdt[0] = CreateDescriptor(0, 0x00000000, 0);            // Null  	 Descriptor
	Gdt[1] = CreateDescriptor(0, 0x000FFFFF, GDT_CODE_PL0); // Kernel Code
	Gdt[2] = CreateDescriptor(0, 0x000FFFFF, GDT_DATA_PL0); // Kernel Data
	// Gdt[3] = CreateDescriptor(0, 0x000FFFFF, GDT_CODE_PL3); // User   Code
	// Gdt[4] = CreateDescriptor(0, 0x000FFFFF, GDT_DATA_PL3); // User   Data

	gp.Limit = sizeof(Gdt) - 1;
	gp.Base = (UINT64)&Gdt;

	extern VOID GdtFlush(CONST struct GdtPtr *gdt);
	GdtFlush(&gp);
}

static SHSTATUS XScopeGdtInit(VOID) {
	GdtInit();

	return STATUS_SUCCESS;
}

XSCOPENODE(X64_GDT, XScopeGdtInit);