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
#include <Int/IO.H>

/* --- Typedefs - Structs - Enums ---*/
#define UART_IRQ	4	// UART's Interrupt Request pin
#define COM1	0x03F8	// UART's base I/O port-address
#define INTR_MASK	0x0F	// UART Interrupt Mask

/* --- Globals ---*/

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

}

VOID SerialWriteByte(
	IN UINT8 Byte
) {

}