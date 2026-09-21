/*
	* Shadow/Include/INT/IO.H - Port I/O primitives
	* Author:   amity
	* Date:     Thu Sep 17 23:26:55 2026
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
#ifndef __INT_IO_H__
#define __INT_IO_H__

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/
FORCEINLINE VOID OutByte(
	IN UINT16 Port,
	IN UINT8 Value
) {
	__asm__ __volatile__("outb %0, %1":: "a"(Value), "Nd"(Port));
}

FORCEINLINE UINT8 InByte(
	IN UINT16 Port
) {
	UINT8 Value;
	__asm__ __volatile__("inb %1, %0" : "=a"(Value) : "Nd"(Port));

	return Value;
}

FORCEINLINE VOID IOWait(VOID) {
	/*
		* Port 0x80 is POST-code diagnostic space on real hardware 
		* unused by anything that matters, so writing to it is the traditional
		* "burn a bus cycle" trick for giving the (comparatively glacial)
		* 8259 time to actually latch a write before the next one lands.
	*/

	OutByte(0x80, 0);
}


#endif /* __INT_IO_H__ */