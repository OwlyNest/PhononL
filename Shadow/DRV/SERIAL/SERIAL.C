/*
	* Shadow/DRV/SERIAL/SERIAL.C - Serial (16550 UART) output
	* Author:   amity
	* Date:     Fri Sep 25 00:40:50 2026
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
#include <DRV/SERIAL/SERIAL.H>
#include <XAL/XScope.H>
#include <Int/IO.H>

/* --- Typedefs - Structs - Enums ---*/
#define UART_IRQ	4	// UART's Interrupt Request pin
#define COM1	0x03F8	// UART's base I/O port-address
#define INTR_MASK	0x0F	// UART Interrupt Mask

enum	{
	UART_RX_DATA	= COM1 + 0,
	UART_TX_DATA	= COM1 + 0,
	UART_DLATCH_LO	= COM1 + 0,
	UART_DLATCH_HI	= COM1 + 1,
	UART_INTR_EN	= COM1 + 1,
	UART_INTR_ID	= COM1 + 2,
	UART_FIFO_CTRL	= COM1 + 2,
	UART_LINE_CTRL	= COM1 + 3,
	UART_MODEM_CTRL	= COM1 + 4,
	UART_LINE_STAT	= COM1 + 5,
	UART_MODEM_STAT = COM1 + 6,
	UART_SCRATCH	= COM1 + 7,
	};

/* --- Globals ---*/
static BOOLEAN SerialPresent = FALSE;

/* --- Prototypes ---*/

/* --- Functions ---*/
static BOOLEAN SerialSelfTest(VOID) {
	OutByte(COM1 + 1, 0x00);   /* IRQs off */
	OutByte(COM1 + 3, 0x80);   /* DLAB on */
	OutByte(COM1 + 0, 0x01);   /* divisor low: 115200 baud */
	OutByte(COM1 + 1, 0x00);   /* divisor high */
	OutByte(COM1 + 3, 0x03);   /* DLAB off, 8N1 */
	OutByte(COM1 + 2, 0xC7);   /* FIFO on, clear, 14-byte threshold */

	OutByte(COM1 + 4, 0x1E);   /* MCR: loopback mode */
	OutByte(COM1 + 0, 0xAE);   /* arbitrary test byte */

	if (InByte(COM1 + 0) != 0xAE) {
		return FALSE;          /* no UART here -- real hardware, no header */
	}

	OutByte(COM1 + 4, 0x0F);   /* loopback off, normal operation */
	return TRUE;
}


SHSTATUS SerialInit(VOID) {
	if (!SerialSelfTest()) {
		return (SHSTATUS)-1;   /* no UART here; SerialWriteByte stays silent */
	}

	// configure the UART
	OutByte(UART_INTR_EN, 0x00);
	OutByte(UART_FIFO_CTRL, 0xC7);
	OutByte(UART_LINE_CTRL, 0x83);
	OutByte(UART_DLATCH_LO, 0x01);
	OutByte(UART_DLATCH_HI, 0x00);
	OutByte(UART_LINE_CTRL, 0x03);
	OutByte(UART_MODEM_CTRL, 0x03);
	InByte(UART_MODEM_STAT );
	InByte(UART_LINE_STAT );
	InByte(UART_RX_DATA );
	InByte(UART_INTR_ID );

	SerialPresent = TRUE;
	return STATUS_SUCCESS;
}

VOID SerialWriteByte(
	IN UINT8 Byte
) {
	if (!SerialPresent) {
		return;
	}

	while (!(InByte(UART_LINE_STAT) & 0x20)) {
		/* wait for THRE: transmit holding register empty */
	}

	OutByte(UART_TX_DATA, Byte);
}

XSCOPENODE(SERIAL, SerialInit);