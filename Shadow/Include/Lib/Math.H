/*
	* Include/Lib/Math.H - [Enter description]
	* Author:   amity
	* Date:     Thu Jul  2 14:53:00 2026
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
#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

#define CORDIC_ITERATIONS   16
#define CORDIC_K_INV        107829

#define FX_SHIFT            16
#define FX_ONE              (1 << FX_SHIFT)
#define FX_MASK             (FX_ONE - 1)
#define FX_FROM_INT(i)      ((INT32)(i) << FX_SHIFT)
#define FX_TO_INT(f)        ((INT32)((f) >> FX_SHIFT))
#define FX_TO_INT_RND(f)    ((INT32)(((f) + (FX_ONE >> 1)) >> FX_SHIFT))
#define FX_MUL(a, b)        ((INT32)(((INT64)(a) * (INT64)(b)) >> FX_SHIFT))
#define FX_DIV(a, b)        ((INT32)((((INT64)(a) << FX_SHIFT) / (INT64)(b))))
#define ANGLE_FULL_TENTHS   3600     /* tenths of degrees */
#define ANGLE_FULL_BRAD     65536    /* bradians: 0x10000 = 360° */
#define BRAD_90             0x4000   /* 16384 */
#define BRAD_180            0x8000   /* 32768 */
#define BRAD_270            0xC000   /* 49152 */

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/
typedef INT32 FX;

typedef struct {
    INT x;
    INT y;
} Vec2I;

typedef struct {
    FX x;
    FX y;
} Vec2F;

/* --- Globals ---*/

/* --- Prototypes ---*/
#endif