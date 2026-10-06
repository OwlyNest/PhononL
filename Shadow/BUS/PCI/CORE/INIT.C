/*
	* Shadow/BUS/PCI/CORE/INIT.C - [Enter description]
	* Author:   amity
	* Date:     Fri Oct  2 21:18:48 2026
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
//#include "../PCI.H"
#include <BUS/PCI/PCI.H>
#include <BUS/PCI/PCI_IDS.H>
#include <Lib/Lib.H>
#include <DRV/PS2/PS2.H>

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/

SHSTATUS PciInit(VOID) {
	printk("[PCI] Initialising subsystem (x86_64)\r\n");
    Kbr();

	PciHostInit();
	Kbr();

	PciMcfgInit();
    Kbr();

	printk("Starting enumeration\r\n");
	Kbr();

	SHSTATUS Rc = PciEnumerate();
	if (Rc < 0) {
		printk("Enumeration failed (%d)\r\n", Rc);
        return Rc;
	}

	printk("[PCI] Enumeration complete\r\n");
    Kbr();

	return STATUS_SUCCESS;
}