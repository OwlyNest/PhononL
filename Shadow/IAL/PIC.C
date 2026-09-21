/*
	* Shadow/IAL/PIC.C - IAL backend for the 8259 PIC
	* Author:   amity
	* Date:     Sat Sep 19 00:06:13 2026
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
#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1
 
#define PIC_ICW1_INIT   0x11   /* edge-triggered, cascade, ICW4 follows */
#define PIC_ICW4_8086   0x01
 
#define PIC_VECTOR_BASE 32     /* master: 32-39, slave: 40-47 */
 
#define PIC_EOI         0x20

/* --- Includes ---*/
#include <Int/IO.H>
#include <IAL/PIC.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Prototypes ---*/
static INT   IALPicInit(IN _PIAL_BACKEND Self);
static UINT8 IALPicGetVectorBase(IN _PIAL_BACKEND Self);
static VOID  IALPicEnableIrq(IN _PIAL_BACKEND Self, IN UINT8 Irq);
static VOID  IALPicDisableIrq(IN _PIAL_BACKEND Self, IN UINT8 Irq);
static VOID  IALPicSendEoi(IN _PIAL_BACKEND Self, IN UINT8 Vector);

/* --- Globals ---*/
static _IAL_BACKEND IALPic = {
	.Name          = "8259 Programmable Interrupt Controller",
	.Init          = IALPicInit,
	.GetVectorBase = IALPicGetVectorBase,
	.EnableIrq     = IALPicEnableIrq,
	.DisableIrq    = IALPicDisableIrq,
	.SendEoi       = IALPicSendEoi,
};

/* --- Functions ---*/

/* ==========================================================================
 * Remap
 *
 * Firmware leaves the PIC at its power-on vector range, 0x08-0x0F on the
 * master; directly on top of CPU exception vectors. An unmasked IRQ0
 * (the legacy timer) firing before this runs would be indistinguishable
 * from a Double Fault. Remapping to 32+ first, before anything is
 * unmasked, is not optional ordering.
 *
 * ICW3's two writes look identical (both "4") but mean different things:
 * to the master it's a bitmask (bit 2 set = "a slave lives on my IRQ2"),
 * to the slave it's its cascade identity (an ordinal, "I am slave 2").
 * Same value, unrelated meaning.
 * Worth not assuming one explains the other
 * ======================================================================= */

static INT IALPicInit(
	IN _PIAL_BACKEND Self
) {
	(VOID)Self;

	UINT8 MasterMask = InByte(PIC_MASTER_DATA);
	UINT8 SlaveMask  = InByte(PIC_SLAVE_DATA);
	(VOID)MasterMask;
	(VOID)SlaveMask;

	OutByte(PIC_MASTER_CMD, PIC_ICW1_INIT);
	IOWait();
	OutByte(PIC_SLAVE_CMD,  PIC_ICW1_INIT);
	IOWait();

	OutByte(PIC_MASTER_DATA, PIC_VECTOR_BASE);
	IOWait();
	OutByte(PIC_SLAVE_DATA,  PIC_VECTOR_BASE + 8);
	IOWait();

	OutByte(PIC_MASTER_DATA, 0x04);   /* slave lives on IRQ2 */
	IOWait();
	OutByte(PIC_SLAVE_DATA,  0x02);   /* my identity is 2    */
	IOWait();
 
	OutByte(PIC_MASTER_DATA, PIC_ICW4_8086);
	IOWait();
	OutByte(PIC_SLAVE_DATA,  PIC_ICW4_8086);
	IOWait();

	/*
		* Mask everything. IalEnableIrq unmasks specific lines as whatever
		* actually wants them (keyboard, PIT, ...) gets wired up
		* nothing fires until something has explicitly asked for it.
	*/
	OutByte(PIC_MASTER_DATA, 0xFF);
	OutByte(PIC_SLAVE_DATA,  0xFF);

	return 0;
}

static UINT8 IALPicGetVectorBase(IN _PIAL_BACKEND Self) {
	(VOID)Self;
	return PIC_VECTOR_BASE;
}

static VOID IALPicEnableIrq(
	IN _PIAL_BACKEND Self,
	IN UINT8 Irq
) {
	(VOID)Self;
 
	UINT16 Port = (Irq < 8) ? PIC_MASTER_DATA : PIC_SLAVE_DATA;
	UINT8  Bit  = (Irq < 8) ? Irq : (Irq - 8);
	UINT8  Mask = InByte(Port);
 
	OutByte(Port, Mask & ~(1 << Bit));
}

static VOID IALPicDisableIrq(
	IN _PIAL_BACKEND Self,
	IN UINT8 Irq
) {
	(VOID)Self;
 
	UINT16 Port = (Irq < 8) ? PIC_MASTER_DATA : PIC_SLAVE_DATA;
	UINT8  Bit  = (Irq < 8) ? Irq : (Irq - 8);
	UINT8  Mask = InByte(Port);
 
	OutByte(Port, Mask | (1 << Bit));
}


/* ==========================================================================
 * EOI
 *
 * IRQ8-15 came through the slave, cascaded into the master via IRQ2 --
 * the master never independently noticed the interrupt, only that its
 * cascade line fired. Both controllers need telling, slave first.
 * Forgetting the master half for a slave-side IRQ is the classic version
 * of this bug: the slave keeps delivering once, then silently stops.
 * ======================================================================= */

static VOID IALPicSendEoi(IN _PIAL_BACKEND Self, IN UINT8 Vector) {
	UINT8 Irq = Vector - Self->GetVectorBase(Self);
 
	if (Irq >= 8) {
		OutByte(PIC_SLAVE_CMD, PIC_EOI);
	}
	OutByte(PIC_MASTER_CMD, PIC_EOI);
}

_PIAL_BACKEND IALPicBackend(VOID) {
	return &IALPic;
}