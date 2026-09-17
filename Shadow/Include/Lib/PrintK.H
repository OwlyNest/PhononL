/*
	* Include/Lib/PrintK.H - [Enter description]
	* Author:   amity
	* Date:     Thu Jun 11 10:01:54 2026
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
	* Empty parameter list:          Use (VOID) instead of ()
	* Statements and declarations:   Max one per line
*/

/* --- Macros ---*/
#ifndef __LIB_PRINTK_H__
#define __LIB_PRINTK_H__

/* --- Includes ---*/
#include <stdarg.h>
/* --- Typedefs - Structs - Enums ---*/
typedef enum {
	LEN_NONE,
	LEN_HH,
	LEN_H,
	LEN_L,
	LEN_LL,
	LEN_Z,
	LEN_T
} LengthModifier;

typedef struct {
    INT left;
    INT zero;
    INT alternate;
    INT plus;
    INT space;

    INT width;
    INT precision;      // -1 means "not specified"

	LengthModifier length;    

    char specifier;
} FormatSpec;
/* --- Globals ---*/

/* --- Prototypes ---*/
INT kvsnprintf(PCHAR buf, size_t size, PCCHAR fmt, va_list args);
INT ksnprintf(PCHAR buf, size_t size, PCCHAR fmt, ...);
VOID printk(PCCHAR fmt, ...);
#endif