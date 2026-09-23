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
#define PIC1  0x20
#define PIC1_COMMAND    PIC1
#define PIC1_DATA      (PIC1 + 1)
#define PIC2   0xA0
#define PIC2_COMMAND    PIC2
#define PIC2_DATA      (PIC2 + 1)
#define PIC_EOI         0x20
 
/* Initialization command word 1 (ICW1) */
#define ICW1_ICW4	    0x01		/* 1=ICW4 is needed, 0=no ICW4 needed */
#define ICW1_SINGLE	    0x02		/* 1=single 8259, 0=cascading 8259's */
#define ICW1_INTERVAL4  0x04		/* 1=4 byte interrupt vectors, 0=8 byte int vectors */
#define ICW1_LEVEL	    0x08		/* 1=level triggered mode, 0=edge triggered mode */
#define ICW1_INIT	    0x10		/* 1=level triggered mode, 0=edge triggered mode */

/* Initialization Command Word 4 (ICW4) */
#define ICW4_8086	    0x01		/* 1 for 80x86 mode, 0 = MCS 80/85 mode */
#define ICW4_AUTO	    0x02		/* 1 = auto EOI, 0=normal EOI */
#define ICW4_BUF_SLAVE	0x08	    /* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C	    /* Buffered mode/master */
#define ICW4_SFNM	    0x10		/* Special fully nested (not) */

#define CASCADE_IRQ     2
 
#define PIC_VECTOR_BASE 32         /* master: 32-39, slave: 40-47 */

#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */
 

/* --- Includes ---*/
#include <Int/IO.H>
#include <IAL/PIC.H>
#include <IAL/IAL.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Prototypes ---*/
static INT   IALPicInit(IN _PIAL_BACKEND Self);
static UINT8 IALPicGetVectorBase(IN _PIAL_BACKEND Self);
static VOID  IALPicEnableIrq(IN _PIAL_BACKEND Self, IN UINT8 Irq);
static VOID  IALPicDisableIrq(IN _PIAL_BACKEND Self, IN UINT8 Irq);
static VOID  IALPicSendEoi(IN _PIAL_BACKEND Self, IN UINT8 Vector);
static VOID  IALPicDisable(IN _PIAL_BACKEND Self);

/* --- Globals ---*/
static _IAL_BACKEND IALPic = {
	.Name          = "8259 Programmable Interrupt Controller",
	.Init          = IALPicInit,
	.GetVectorBase = IALPicGetVectorBase,
	.EnableIrq     = IALPicEnableIrq,
	.DisableIrq    = IALPicDisableIrq,
	.SendEoi       = IALPicSendEoi,
	.Disable       = IALPicDisable,
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
 * more directly:
 	│7│6│5│4│3│2│1│0│  ICW3 for Master Device
	 │ │ │ │ │ │ │ └──── 1=interrupt request 0 has slave, 0=no slave
	 │ │ │ │ │ │ └───── 1=interrupt request 1 has slave, 0=no slave
	 │ │ │ │ │ └────── 1=interrupt request 2 has slave, 0=no slave
	 │ │ │ │ └─────── 1=interrupt request 3 has slave, 0=no slave
	 │ │ │ └──────── 1=interrupt request 4 has slave, 0=no slave
	 │ │ └───────── 1=interrupt request 5 has slave, 0=no slave
	 │ └────────── 1=interrupt request 6 has slave, 0=no slave
	 └─────────── 1=interrupt request 7 has slave, 0=no slave

	│7│6│5│4│3│2│1│0│  ICW3 for Slave Device
	 │ │ │ │ │ └─┴─┴──── master interrupt request slave is attached to
	 └─┴─┴─┴─┴───────── must be zero
 * ======================================================================= */

 /*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
static VOID PicRemap(
	IN UINT8 Offset1,
	IN UINT8 Offset2
) {
	OutByte(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // starts the initialization sequence (in cascade mode)
	IOWait();
	OutByte(PIC1_DATA, Offset1);                  // ICW2: Master PIC vector offset
	IOWait();
	OutByte(PIC2_DATA, Offset2);                  // ICW2: Slave PIC vector offset
	IOWait();
	OutByte(PIC1_DATA, 1 << CASCADE_IRQ);         // ICW3: tell Master PIC that there is a slave PIC at IRQ2
	IOWait();
	OutByte(PIC2_DATA, CASCADE_IRQ);              // ICW3: tell Slave PIC its cascade identity
	IOWait();
	
	OutByte(PIC1_DATA, ICW4_8086);                // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	IOWait();
	OutByte(PIC2_DATA, ICW4_8086);
	IOWait();

	// unmask both PICs
	OutByte(PIC1_DATA, 0xFF);
	OutByte(PIC2_DATA, 0xFF);
}

static INT IALPicInit(
	IN _PIAL_BACKEND Self
) {
	(VOID)Self;

	PicRemap(PIC_VECTOR_BASE, PIC_VECTOR_BASE + 8);

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
	UINT16 Port;
	UINT8  Value;

	if (Irq < 8) {
		Port = PIC1_DATA;
	} else {
		Port = PIC2_DATA;
		Irq -= 8;
	}

	Value = InByte(Port) & ~(1 << Irq);   // clear the mask bit
	OutByte(Port, Value);
}

static VOID IALPicDisableIrq(
	IN _PIAL_BACKEND Self,
	IN UINT8 Irq
) {
	(VOID)Self;
	UINT16 Port;
	UINT8  Value;

	if (Irq < 8) {
		Port = PIC1_DATA;
	} else {
		Port = PIC2_DATA;
		Irq -= 8;
	}

	Value = InByte(Port) | (1 << Irq);    // set the mask bit
	OutByte(Port, Value);
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

static VOID IALPicSendEoi(
	IN _PIAL_BACKEND Self,
	IN UINT8 Vector
) {
	UINT8 Irq = Vector - Self->GetVectorBase(Self);
 
	if (Irq >= 8) {
		OutByte(PIC2_COMMAND, PIC_EOI);
	}
	OutByte(PIC1_COMMAND, PIC_EOI);
}

static VOID IALPicDisable(
	IN _PIAL_BACKEND Self
) {
	(VOID)Self;
	OutByte(PIC1_DATA, 0xFF);
	OutByte(PIC2_DATA, 0xFF);
}

_PIAL_BACKEND IALPicBackend(VOID) {
	return &IALPic;
}

// static UINT16 PicGetIrqReg__(
// 	IN UINT8 Ocw3) {
// 	/*
// 		* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
//     	* represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain
// 	*/

// 	OutByte(PIC1_COMMAND, Ocw3);
// 	OutByte(PIC2_COMMAND, Ocw3);

// 	return (InByte(PIC2_COMMAND) << 8) | InByte(PIC1_COMMAND);
// }

// /* Returns the combined value of the cascaded PICs irq request register */
// static UINT16 PicGetIrr(void) {
//     return PicGetIrqReg__(PIC_READ_IRR);
// }

// /* Returns the combined value of the cascaded PICs in-service register */
// static UINT16 PicGetIsr(VOID){
//     return PicGetIrqReg__(PIC_READ_ISR);
// }