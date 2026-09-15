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
#define FX_FROM_INT(i)      ((int32_t)(i) << FX_SHIFT)
#define FX_TO_INT(f)        ((int32_t)((f) >> FX_SHIFT))
#define FX_TO_INT_RND(f)    ((int32_t)(((f) + (FX_ONE >> 1)) >> FX_SHIFT))
#define FX_MUL(a, b)        ((int32_t)(((int64_t)(a) * (int64_t)(b)) >> FX_SHIFT))
#define FX_DIV(a, b)        ((int32_t)((((int64_t)(a) << FX_SHIFT) / (int64_t)(b))))
#define ANGLE_FULL_TENTHS   3600     /* tenths of degrees */
#define ANGLE_FULL_BRAD     65536    /* bradians: 0x10000 = 360° */
#define BRAD_90             0x4000   /* 16384 */
#define BRAD_180            0x8000   /* 32768 */
#define BRAD_270            0xC000   /* 49152 */

/* --- Includes ---*/
#include <stdint.h>
/* --- Typedefs - Structs - Enums ---*/
typedef INT32 FX;

typedef struct {
    INT x;
    INT y;
} vec2i_t;

typedef struct {
    FX x;
    FX y;
} vec2f_t;

/* --- Globals ---*/

/* --- Prototypes ---*/
/* Fixed-point basics */
FX fx_from_int(INT32 i);
INT32 fx_to_int(FX f);
INT32 fx_to_int_rnd(FX f);
FX fx_mul(FX a, FX b);
FX fx_div(FX a, FX b);
FX fx_sqrt(FX x);

/* CORDIC trig: angle in bradians (0-65535, 0x10000 = 360°) */
FX fx_sin_b(INT angle_bradians);
FX fx_cos_b(INT angle_bradians);

/* Angle conversion */
INT angle_to_bradians(INT tenths_of_degrees);

/* Vector helpers */
vec2i_t vec2i(INT x, INT y);
vec2i_t vec2i_add(vec2i_t a, vec2i_t b);
vec2i_t vec2i_sub(vec2i_t a, vec2i_t b);
vec2i_t vec2i_polar_bradians(INT cx, INT cy, INT radius, INT angle_bradians);

/* Integer distance */
int32_t idist(INT x0, INT y0, INT x1, INT y1);
int32_t idist_sq(INT x0, INT y0, INT x1, INT y1);

/* Clamp */
INT32 iclamp(INT32 v, INT32 lo, INT32 hi);
UINT32 uclamp(UINT32 v, UINT32 lo, UINT32 hi);
#endif