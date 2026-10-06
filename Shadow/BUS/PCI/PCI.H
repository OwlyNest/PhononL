/*
	* Shadow/BUS/PCI/PCI.H - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 16:12:56 2026
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
#ifndef __PCI_H__
#define __PCI_H__

#define PCI_DEVFN(slot, func)   ((((slot) & 0x1f) << 3) | ((func) & 0x07))
#define PCI_DEV(devfn)          (((devfn) >> 3) & 0x1f)
#define PCI_FUNC(devfn)         ((devfn) & 0x07)   

#define PCI_IS_BRIDGE(Hdr)      ((Hdr & 0x7F) == 0x01)

#define PCI_IO_WINDOW_START     0x1000ULL
#define PCI_IO_WINDOW_END       0xE000ULL

#define PCI_MEM32_START         0x80000000ULL
#define PCI_MEM32_END           0xE0000000ULL

#define PCI_MEM64_START         0x100000000ULL
#define PCI_MEM64_END           0x140000000ULL

#define PCI_REG_COMMAND     0x04
#define PCI_REG_BAR0        0x10

#define PCI_CMD_IO          0x0001U
#define PCI_CMD_MEM         0x0002U

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/

#endif /* __PCI_H__ */
