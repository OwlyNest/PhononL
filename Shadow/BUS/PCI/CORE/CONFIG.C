/*
	* Shadow/BUS/PCI/Core/CONFIG.C - [Enter description]
	* Author:   amity
	* Date:     Thu Oct  1 16:13:48 2026
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
#include <BUS/PCI/PCI.H>
#include <Lib/Lib.H>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
static _PciOps CurrentOps;
/* --- Prototypes ---*/

/* --- Functions ---*/
VOID PciConfigOpsRegister(
	IN _PPciOps Ops
) {
	if (Ops == NULL) {
		printk("ConfigOpsRegister: NULL Ops\r\n");
		return;
	}
	CurrentOps = *Ops;
	printk("Config ops registered\r\n");
}


SHSTATUS PciReadConfig(
	IN _PPciDev Dev,
	IN UINT Offset,
	IN UINT Size,
	OUT PVOID Val
) {
	if (Dev == NULL || Val == NULL) {
		return (SHSTATUS)-1;
	}

	if (CurrentOps.Read == NULL) {
		return (SHSTATUS)-1;
	}

	return (SHSTATUS)CurrentOps.Read(Dev, Offset, Size, Val);
}

SHSTATUS PciWriteConfig(
	IN _PPciDev Dev,
	IN UINT Offset,
	IN UINT Size,
	IN UINT64 Val
) {
	if (Dev == NULL) {
		return (SHSTATUS)-1;
	}

	if (CurrentOps.Write == NULL) {
		return (SHSTATUS)-1;
	}

	return (SHSTATUS)CurrentOps.Write(Dev, Offset, Size, Val);
}