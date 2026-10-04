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
FORCEINLINE UINT32 InData(
    IN UINT16 Port,
    IN SIZE_T Size
) {
    switch (Size) {
        case 1: {
            UINT8 Value;

            __asm__ __volatile__(
                "inb %1, %0"
                : "=a"(Value)
                : "Nd"(Port)
            );

            return Value;
        }

        case 2: {
            UINT16 Value;

            __asm__ __volatile__(
                "inw %1, %0"
                : "=a"(Value)
                : "Nd"(Port)
            );

            return Value;
        }

        case 4: {
            UINT32 Value;

            __asm__ __volatile__(
                "inl %1, %0"
                : "=a"(Value)
                : "Nd"(Port)
            );

            return Value;
        }

        default: {
            __builtin_unreachable();
		}
    }
}

FORCEINLINE VOID OutData(
    IN UINT16 Port,
    IN SIZE_T Size,
    IN UINT32 Value
) {
    switch (Size) {
        case 1: {
            UINT8 Data = (UINT8)Value;

            __asm__ __volatile__(
                "outb %0, %1"
                :
                : "a"(Data), "Nd"(Port)
            );

            break;
        }

        case 2: {
            UINT16 Data = (UINT16)Value;

            __asm__ __volatile__(
                "outw %0, %1"
                :
                : "a"(Data), "Nd"(Port)
            );

            break;
        }

        case 4: {
            __asm__ __volatile__(
                "outl %0, %1"
                :
                : "a"(Value), "Nd"(Port)
            );

            break;
        }

        default: {
            __builtin_unreachable();
		}
    }
}


#define InByte(Port) InData(Port, 1)
#define InWord(Port) InData(Port, 2)
#define InLong(Port) InData(Port, 4)   

#define OutByte(Port, Value) OutData(Port, 1, Value);
#define OutWord(Port, Value) OutData(Port, 2, Value);
#define OutLong(Port, Value) OutData(Port, 4, Value);

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