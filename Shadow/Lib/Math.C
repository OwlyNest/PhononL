/*
	* lib/math.c - [Enter description]
	* Author:   amity
	* Date:     Thu Jul  2 14:52:55 2026
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
#include <stdint.h>
#include <Lib/Math.H>

/* --- Typedefs - Structs - Enums ---*/
static const INT32 cordic_atan_table[CORDIC_ITERATIONS] = {
    0x2000,  /* 8192   = 45.000° */
    0x12E4,  /* 4836   = 26.565° */
    0x09FB,  /* 2555   = 14.036° */
    0x0511,  /* 1297   =  7.125° */
    0x028B,  /*  651   =  3.576° */
    0x0145,  /*  325   =  1.790° */
    0x00A2,  /*  162   =  0.895° */
    0x0051,  /*   81   =  0.448° */
    0x0028,  /*   40   =  0.224° */
    0x0014,  /*   20   =  0.112° */
    0x000A,  /*   10   =  0.056° */
    0x0005,  /*    5   =  0.028° */
    0x0002,  /*    2   =  0.014° */
    0x0001,  /*    1   =  0.007° */
    0x0001,  /*    1   =  0.007° */
    0x0000   /*    0   =  0.003° (below resolution) */
};
/* --- Globals ---*/

/* --- Prototypes ---*/
static VOID cordic_rotate(INT32 *x, INT32 *y, INT32 angle);
/* --- Functions ---*/

/* ========================================================================== *
 *                                                                            *
 * CORDIC in rotation mode.                                                   *
 *                                                                            *
 * Input:  angle in bradians (0x10000 = 360°)                                 *
 * Output: *x = cos(angle) in 16.16 fixed point                               *
 *         *y = sin(angle) in 16.16 fixed point                               *
 *                                                                            *
 * We start with x = 1/K, y = 0, z = angle.                                   *
 * After iterations, x ≈ cos(angle), y ≈ sin(angle).                          *
 *                                                                            *
 * Pre-scaling by 1/K means no post-multiply needed.                          *
 *                                                                            *
 * ========================================================================== */
static VOID cordic_rotate(INT32 *x, INT32 *y, INT32 angle) {
    INT32 xi = CORDIC_K_INV;  /* x = 1/K */
    INT32 yi = 0;             /* y = 0 */
    INT32 zi = angle;         /* z = target angle */

    for (INT i = 0; i < CORDIC_ITERATIONS; i++) {
        INT32 x_shift = xi >> i;
        INT32 y_shift = yi >> i;

        if (zi < 0) {
            /* Rotate clockwise */
            xi = xi + y_shift;
            yi = yi - x_shift;
            zi = zi + cordic_atan_table[i];
        } else {
            /* Rotate counter-clockwise */
            xi = xi - y_shift;
            yi = yi + x_shift;
            zi = zi - cordic_atan_table[i];
        }
    }

    *x = xi;
    *y = yi;
}

/* ==========================================================================
 * Sin/Cos using CORDIC
 *
 * Angle: bradians (0 to 0xFFFF, where 0x10000 = 360°)
 * Returns: 16.16 fixed point (-1.0 to +1.0)
 * ======================================================================= */
FX fx_sin_b(INT angle) {
    /* Normalize to 0-0xFFFF */
    angle = angle & 0xFFFF;

    /* CORDIC works best in -90° to +90° range (-0x4000 to +0x4000).
     * Use symmetry for other quadrants:
     *   Q1 (0-90°):   sin(a) = +sin(a)
     *   Q2 (90-180°): sin(a) = +sin(180°-a)
     *   Q3 (180-270°):sin(a) = -sin(a-180°)
     *   Q4 (270-360°):sin(a) = -sin(360°-a)
     */

    INT32 x, y;
    INT sign = 1;
    INT cordic_angle;

    if (angle < 0x4000) {
        /* Q1: 0-90° */
        cordic_angle = angle;
    } else if (angle < 0x8000) {
        /* Q2: 90-180° */
        cordic_angle = 0x8000 - angle;
    } else if (angle < 0xC000) {
        /* Q3: 180-270° */
        cordic_angle = angle - 0x8000;
        sign = -1;
    } else {
        /* Q4: 270-360° */
        cordic_angle = 0x10000 - angle;
        sign = -1;
    }

    cordic_rotate(&x, &y, cordic_angle);

    /* y is sin, in 16.16 fixed point. Apply quadrant sign. */
    return sign * y;
}

FX fx_cos_b(INT angle) {
    /* cos(a) = sin(a + 90°) = sin(a + 0x4000) */
    return fx_sin_b(angle + 0x4000);
}

/* ==========================================================================
 * Convert tenths-of-degrees to bradians
 * ======================================================================= */
INT angle_to_bradians(INT tenths_of_degrees) {
    /* 3600 tenths = 0x10000 bradians
     * tenths * 0x10000 / 3600 = tenths * 0x10000 / 3600
     * = tenths * 65536 / 3600
     * = tenths * 8192 / 450
     * = tenths * 4096 / 225
     *
     * 4096/225 ≈ 18.2044, so:
     * bradians = (tenths * 4096) / 225
     *
     * To avoid overflow for large tenths: use 64-bit intermediate.
     */
     return (tenths_of_degrees * 4096) / 225;
}

/* ==========================================================================
 * Vector endpoint from polar coordinates
 * (cx, cy) = center, radius = length, angle = bradians
 * ======================================================================= */
vec2i_t vec2i_polar_bradians(INT cx, INT cy, INT radius, INT angle) {
    INT angle_bradians = angle_to_bradians(angle);
    FX s = fx_sin_b(angle_bradians);
    FX c = fx_cos_b(angle_bradians);

    vec2i_t v;
    v.x = cx + fx_to_int_rnd(fx_mul(c, fx_from_int(radius)));
    v.y = cy - fx_to_int_rnd(fx_mul(s, fx_from_int(radius)));
    return v;
}

/* ==========================================================================
 * Integer square root (Newton / binary search — unchanged)
 * ======================================================================= */
FX fx_sqrt(FX x) {
    if (x <= 0) return 0;

    UINT32 v = (uint32_t)x;
    UINT32 res = 0;
    UINT32 bit = 1u << 30;

    while (bit > v) bit >>= 2;

    while (bit != 0) {
        if (v >= res + bit) {
            v -= res + bit;
            res = (res >> 1) + bit;
        } else {
            res >>= 1;
        }
        bit >>= 2;
    }

    return (FX)(res << 8);
}

/* ==========================================================================
 * Fixed-point basics (unchanged)
 * ======================================================================= */
FX fx_from_int(INT32 i) {
    return (FX)(i << FX_SHIFT);
}

INT32 fx_to_int(FX f) {
    return f >> FX_SHIFT;
}

INT32 fx_to_int_rnd(FX f) {
    return (f + (FX_ONE >> 1)) >> FX_SHIFT;
}

FX fx_mul(FX a, FX b) {
    return (FX)(((int64_t)a * (int64_t)b) >> FX_SHIFT);
}

FX fx_div(FX a, FX b) {
    if (b == 0) return 0;
    return (FX)(((int64_t)a << FX_SHIFT) / (int64_t)b);
}

/* ==========================================================================
 * Vector helpers (integer)
 * ======================================================================= */
vec2i_t vec2i(INT x, INT y) {
    vec2i_t v = { x, y };
    return v;
}

vec2i_t vec2i_add(vec2i_t a, vec2i_t b) {
    vec2i_t v = { a.x + b.x, a.y + b.y };
    return v;
}

vec2i_t vec2i_sub(vec2i_t a, vec2i_t b) {
    vec2i_t v = { a.x - b.x, a.y - b.y };
    return v;
}

/* ==========================================================================
 * Distance
 * ======================================================================= */
INT32 idist_sq(INT x0, INT y0, INT x1, INT y1) {
    INT32 dx = (int32_t)(x1 - x0);
    INT32 dy = (int32_t)(y1 - y0);
    return dx * dx + dy * dy;
}

INT32 idist(INT x0, INT y0, INT x1, INT y1) {
    return fx_to_int(fx_sqrt(idist_sq(x0, y0, x1, y1)));
}

/* ==========================================================================
 * Clamp
 * ======================================================================= */
INT32 iclamp(INT32 v, INT32 lo, INT32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

UINT32 uclamp(UINT32 v, UINT32 lo, UINT32 hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}