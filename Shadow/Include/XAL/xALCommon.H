/*
	* Shadow/Include/XAL/xALCommon.H - [Enter description]
	* Author:   amity
	* Date:     Sun Sep 20 13:34:14 2026
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

/*
	* The one place the token-paste indirection trick is defined. XMcal.H
	* and XScope.H both need it for the same reason (building a name out
	* of a subsystem-supplied prefix/identifier) and neither should carry
	* its own copy; that's the exact kind of accidental duplication the
	* xAL formalization exists to remove.
	*
	* Ordinary header guard: unlike XMcal.H, this file is meant to be
	* included normally, once per translation unit however many times it
	* is reached.
*/

/* --- Macros ---*/
#ifndef __XAL_XALCOMMON_H__
#define __XAL_XALCOMMON_H__

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

#define XAL_CONCAT_(a, b) a##b
#define XAL_CONCAT(a, b)  XAL_CONCAT_(a, b)
 
#endif /* __XAL_XALCOMMON_H__ */