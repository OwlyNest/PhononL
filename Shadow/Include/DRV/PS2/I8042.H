/*
	* Shadow/Include/DRV/PS2/I8042.H - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 28 02:12:20 2026
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
#ifndef __DRV_PS2_I8042_H__
#define __DRV_PS2_I8042_H__

/*
	* PS/2 Controller IO Ports
	* The PS/2 Controller itself uses 2 IO ports (usually, IO ports 0x60 and 0x64). Like many IO ports, reads and writes may access different internal registers.
	* 
	* Historical note: The PC-XT PPI had used port 0x61 to reset the keyboard interrupt request signal (among other unrelated functions). Port 0x61 has no keyboard related functions on AT and PS/2 compatibles.
	* 
	* | IO Port |	Access Type | Purpose          |
	* | 0x60    |	Read/Write  | Data Port        |
	* | 0x64    |	Read	    | Status Register  |
	* | 0x64    |	Write	    | Command Register |
	* 
*/
/* For I/O */
#define PS2_DATA_PORT 0x60
/* For InByte */
#define PS2_STATUS_REGISTER 0x64
/* For OutByte */
#define PS2_COMMAND_REGISTER 0x64
/*
	* Status Register
	* The Status Register contains various flags that show the state of the PS/2 controller. The meanings for each bit are:
	* 
	* | Bit | Meaning  																																	       |
	* | 0	  | Output buffer status (0 = empty, 1 = full) (must be set before attempting to read data from IO port 0x60)   								   |
	* | 1	  | Input buffer status (0 = empty, 1 = full). (must be clear before attempting to write data to IO port 0x60 or IO port 0x64)  				   |
	* | 2	  | System Flag. Meant to be cleared on reset and set by firmware (via. PS/2 Controller Configuration Byte) if the system passes self tests (POST) |
	* | 3	  | Command/data (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command)  |
	* | 4	  | Unknown (chipset specific) May be "keyboard lock" (more likely unused on modern systems)                                                       |
	* | 5	  | Unknown (chipset specific). May be "receive time-out" or "second PS/2 port output buffer full"                                                 |
	* | 6	  | Time-out error (0 = no error, 1 = time-out error) 																							   |
	* | 7	  | Parity error (0 = no error, 1 = parity error)             																					   |
*/
#define PS2_SR_OUTPUT_BUFFER_STATUS (1 << 0)
#define PS2_SR_INPUT_BUFFER_STATUS  (1 << 1)
#define PS2_SR_SYSTEM_FLAG          (1 << 2)
#define PS2_SR_COMMAND_DATA         (1 << 3)
#define PS2_SR_CHIP_SPEC1           (1 << 4)
#define PS2_SR_CHIP_SPEC2           (1 << 5)
#define PS2_SR_TIME_OUT_ERROR       (1 << 6)
#define PS2_SR_PARITY_ERROR         (1 << 7)


/*
	* The PS/2 Controller accepts commands and performs them. These commands should not be confused with bytes sent to a PS/2 device (e.g. keyboard, mouse).
	* To send a command to the controller, write the command byte to IO port 0x64. If the command is 2 bytes long, then the next byte needs to be written to IO Port 0x60 after making sure that the controller is ready for it (by making sure bit 1 of the Status Register is clear). If there is a response byte, then the response byte needs to be read from IO Port 0x60 after making sure it has arrived (by making sure bit 0 of the Status Register is set).
	*
	* | Command      | Meaning 																																																														| Response 																													|
	* | 0x20         | Read "byte 0" from internal RAM 																																																								| Controller Configuration Byte (see below) 																				|
	* | 0x21 to 0x3F | Read "byte N" from internal RAM (where 'N' is the command byte & 0x1F) 																																														| Unknown (only the first byte of internal RAM has a standard purpose) 														|
	* | 0x60         | Write next byte to "byte 0" of internal RAM (Controller Configuration Byte, see below) 																																										| None 																														|
	* | 0x61 to 0x7F | Write next byte to "byte N" of internal RAM (where 'N' is the command byte & 0x1F) 																																											| None 																														|
	* | 0xA7         | Disable second PS/2 port (only if 2 PS/2 ports supported) 																																																	| None 																														|
	* | 0xA8         | Enable second PS/2 port (only if 2 PS/2 ports supported) 																																																	| None 																														|
	* | 0xA9         | Test second PS/2 port (only if 2 PS/2 ports supported) 																																																		| 0x00 test passed, 0x01 clock line stuck low 0x02 clock line stuck high 0x03 data line stuck low 0x04 data line stuck high | 
	* | 0xAA         | Test PS/2 Controller 																																																										| 0x55 test passed, 0xFC test failed 																						|
	* | 0xAB         | Test first PS/2 port 																																																										| 0x00 test passed, 0x01 clock line stuck low 0x02 clock line stuck high 0x03 data line stuck low 0x04 data line stuck high |
	* | 0xAC         | Diagnostic dump (read all bytes of internal RAM) 																																																			| Unknown 																													|
	* | 0xAD         | Disable first PS/2 port 																																																										| None 																														|
	* | 0xAE         | Enable first PS/2 port 																																																										| None 																														|
	* | 0xC0         | Read controller input port 																																																									| Unknown (none of these bits have a standard/defined purpose) 																|
	* | 0xC1         | Copy bits 0 to 3 of input port to status bits 4 to 7 																																																		| None 																														|
	* | 0xC2         | Copy bits 4 to 7 of input port to status bits 4 to 7 																																																		| None 																														|
	* | 0xD0         | Read Controller Output Port 																																																									| Controller Output Port (see below) 																						| 
	* | 0xD1         | Write next byte to Controller Output Port (see below). Note: Check if output buffer is empty first                                                                                                                                                           | None 																														|
	* | 0xD2         | Write next byte to first PS/2 port output buffer (only if 2 PS/2 ports supported), (makes it look like the byte written was received from the first PS/2 port)                                                                                               | None 																														|
	* | 0xD3         | Write next byte to second PS/2 port output buffer (only if 2 PS/2 ports supported),(makes it look like the byte written was received from the second PS/2 port) 																								| None 																														|
	* | 0xD4         | Write next byte to second PS/2 port input buffer (only if 2 PS/2 ports supported), (sends next byte to the second PS/2 port) 																																| None 																														|
	* | 0xF0 to 0xFF | Pulse output line low for 6 ms. Bits 0 to 3 are used as a mask (0 = pulse line, 1 = don't pulse line) and correspond to 4 different output lines. Note: Bit 0 corresponds to the "reset" line. The other output lines don't have a standard/defined purpose. | None 																														|
*/
#define PS2_CMD_READ_CFG_BYTE       0x20
#define PS2_CMD_READ_RAM_BYTE(N)    (0x20 | ((N) & 0x1F))
#define PS2_CMD_WRITE_CFG_BYTE      0x60
#define PS2_CMD_WRITE_RAM_BYTE(N)   (0x60 | ((N) & 0x1F))
#define PS2_CMD_DISABLE_PORT2       0xA7
#define PS2_CMD_ENABLE_PORT2        0xA8
#define PS2_CMD_TEST_PORT2          0xA9
#define PS2_CMD_TEST_CONTROLLER     0xAA
#define PS2_CMD_TEST_PORT1          0xAB
#define PS2_CMD_DIAG_DUMP           0xAC
#define PS2_CMD_DISABLE_PORT1       0xAD
#define PS2_CMD_ENABLE_PORT1        0xAE
#define PS2_CMD_READ_INPUT_PORT     0xC0
#define PS2_CMD_COPY_IN_LOW_TO_SR   0xC1
#define PS2_CMD_COPY_IN_HIGH_TO_SR  0xC2
#define PS2_CMD_READ_OUTPUT_PORT    0xD0
#define PS2_CMD_WRITE_OUTPUT_PORT   0xD1
#define PS2_CMD_WRITE_PORT1_BUF     0xD2
#define PS2_CMD_WRITE_PORT2_BUF     0xD3
#define PS2_CMD_SEND_PORT2          0xD4
#define PS2_CMD_PULSE_LINE(M)       (0xF0 | ((M) & 0x0F))

/*
	*PS/2 Controller Configuration Byte
	*Commands 0x20 and 0x60 let you read and write the PS/2 Controller Configuration Byte. This configuration byte has the following format:
	*
	* | Bit | Meaning                                                                                |
	* | 0   | First PS/2 port interrupt (1 = enabled, 0 = disabled)                                  |
	* | 1   | Second PS/2 port interrupt (1 = enabled, 0 = disabled, only if 2 PS/2 ports supported) |
	* | 2   | System Flag (1 = system passed POST, 0 = your OS shouldn't be running)                 |
	* | 3   | Should be zero                                                                         |
	* | 4   | First PS/2 port clock (1 = disabled, 0 = enabled)                                      |
	* | 5   | Second PS/2 port clock (1 = disabled, 0 = enabled, only if 2 PS/2 ports supported)     |
	* | 6   | First PS/2 port translation (1 = enabled, 0 = disabled)                                |
	* | 7   | Must be zero                                                                           |
	* 
	* Note: Bits that are marked in the table above as "only if 2 PS/2 ports supported" should be treated as either chipset specific or unknown if the controller only supports one PS/2 port.
*/
#define PS2_CCB_PORT1_INT   (1 << 0)
#define PS2_CCB_PORT2_INT   (1 << 1)
#define PS2_CCB_SYSTEM_FLAG (1 << 2)
#define PS2_CCB_Z1          (1 << 3)
#define PS2_CCB_PORT1_CLOCK (1 << 4)
#define PS2_CCB_PORT2_CLOCK (1 << 5)
#define PS2_CCB_PORT1_TRANS (1 << 6) /* ✨🏳️‍⚧️🦄🏳️‍⚧️✨ */
#define PS2_CCB_Z2          (1 << 7)

/*
	* PS/2 Controller Output Port
	* Commands 0xD0 and 0xD1 let you read and write the PS/2 Controller Output Port. This output port has the following format:
	* 
	* | Bit | Meaning 																																											   |
	* | 0   | System reset (output). WARNING always set to '1'. You need to pulse the reset line (e.g. using command 0xFE), and setting this bit to '0' can lock the computer up ("reset forever") |
	* | 1   | A20 gate (output) 																																								   |
	* | 2   | Second PS/2 port clock (output, only if 2 PS/2 ports supported)                                                                                                                      |
	* | 3   | Second PS/2 port data (output, only if 2 PS/2 ports supported)                                                                                                                       |
	* | 4   | Output buffer full with byte from first PS/2 port (connected to IRQ1)                                                                                                                |
	* | 5   | Output buffer full with byte from second PS/2 port (connected to IRQ12, only if 2 PS/2 ports supported)                                                                              |
	* | 6   | First PS/2 port clock (output)                                                                                                                                                       |
	* | 7   | First PS/2 port data (output)                                                                                                                                                        |
*/
#define PS2_COP_SYSTEM_RESET   (1 << 0)
#define PS2_COP_A20            (1 << 1)
#define PS2_COP_PORT2_CLOCK    (1 << 2)
#define PS2_COP_PORT2_DATA     (1 << 3)
#define PS2_COP_PORT1_OUT_FULL (1 << 4)
#define PS2_COP_PORT2_OUT_FULL (1 << 5)
#define PS2_COP_PORT1_CLOCK    (1 << 6)
#define PS2_COP_PORT1_DATA     (1 << 7)

/* --- Device Identify (device cmd 0xF2, translated responses) ---*/
#define PS2_ID_MOUSE            0x00 /* Standard PS/2 mouse       */
#define PS2_ID_MOUSE_SCROLL     0x03 /* Scroll wheel mouse        */
#define PS2_ID_MOUSE_5BTN       0x04 /* 5-button mouse            */
#define PS2_ID_KB_PREFIX        0xAB /* Keyboard first ID byte    */
#define PS2_ID_KB_MF2           0x41 /* MF2 keyboard (translated) */
#define PS2_ID_KB_MF2_ALT       0xC1 /* MF2 keyboard variant      */

/*
	* Spin bound for the input buffer. Iterations are tens of ns; the I8042
	* normally clears IBF within microseconds, so this is generous.
*/
#define I8042_SPIN_TIMEOUT 0x00010000
/*
	*Device replies must cross the wire again (~1 ms at 10-16.7 kHz);
	* an order of magnitude more spins than I8042_SPIN_TIMEOUT, which is
	* tuned for the 8042 itself clearing IBF within microseconds.
*/
#define I8042_RESP_TIMEOUT 0x01000000

/* 
	* Device handshake: how many times a RESEND (0xFE) is honoured
	* before giving up. The byte is sent at most MAX+1 times.
*/
#define I8042_MAX_RESENDS 3

/* --- Device Handshake (universal device replies) ---*/
#define PS2_DEV_ACK    0xFA /* Command received and valid            */
#define PS2_DEV_RESEND 0xFE /* Frame corrupt: send the byte again    */
#define PS2_DEV_ERR0   0xFC /* Internal device error: no retry       */
#define PS2_DEV_ERR1   0xFD /* Internal device error: no retry       */

/* Error codes, SmKFS G1 standard */
typedef enum {
	PS2_SUCCESS               = 0,
	PS2_ERR_INVALID_PARAMETER = -1,
	PS2_ERR_DEVICE            = -2,
	/*
		* Add more at need
	*/

} _PS2_Status, *_PPS2_Status;

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

#endif /* __DRV_PS2_I8042_H__ */