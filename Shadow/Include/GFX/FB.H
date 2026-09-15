/*
	* Shadow/Include/GFX/FB.H - [Enter description]
	* Author:   amity
	* Date:     Mon Sep 14 12:42:04 2026
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
#ifndef __GFX_FB_H__
#define __GFX_FB_H__

/* --- Includes ---*/

/* --- Typedefs - Structs - Enums ---*/
typedef struct GFX_SURFACE {
	UINT32 *Pixels;
	UINT32  Width;
	UINT32  Height;
	UINT32  Pitch;
	UINT32  PitchPx;
} _GFX_SURFACE, *_PGFX_SURFACE;

typedef struct FB_SURFACE {
	_GFX_SURFACE  Back;
	UINT32       *Front;
	INT			  Initialized;
} _FB_SURFACE, *_PFB_SURFACE;

typedef enum {
	GFX_BG_DESKTOP,
	GFX_BG_PANEL,
	GFX_BG_TITLE,
  	GFX_BG_HIGHLIGHT,
  	GFX_BG_BUTTON,
  	GFX_BG_BUTTON_HOVER,
  	GFX_FG_TEXT,
  	GFX_FG_TEXT_DIM,
  	GFX_FG_ACCENT,
  	GFX_BORDER_LIGHT,
  	GFX_BORDER_DARK,
	GFX_RED,
	GFX_GREEN,
	GFX_BLUE,
	GFX_YELLOW,
	GFX_WHITE,
	GFX_BLACK,
} _GFX_THEME_COLOR, *_PGFX_THEME_COLOR;

/*
 * Forward-declared to avoid a circular include with GAL.H: FB.H only
 * ever needs a pointer to a backend here, GAL.H holds the full
 * definition (which itself includes FB.H, for _GFX_SURFACE).
*/
typedef struct GAL_BACKEND _GAL_BACKEND, *_PGAL_BACKEND;

/* --- Globals ---*/
extern _FB_SURFACE fb;
/* --- Prototypes ---*/
VOID fb_set_backend(IN _PGAL_BACKEND backend);

INT fb_init(VOID);
VOID fb_update_hw(VOID);
VOID fb_present(VOID); /* Copy backbuffer to screen and update */

UINT32 fb_pack_pixel(IN UINT8 r, IN UINT8 g, IN UINT8 b);
UINT32 gfx_theme_color(IN _GFX_THEME_COLOR c);

/* Generic drawing */
 
VOID gfx_fill_circle(_PGFX_SURFACE surface, INT cx, INT cy, INT radius, UINT32 color);
VOID gfx_draw_line_thick(_PGFX_SURFACE surface, INT x0, INT y0, INT x1, INT y1, INT thickness, UINT32 color);
VOID gfx_draw_vector(_PGFX_SURFACE surface, INT x0, INT y0, INT angle, INT magnitude, INT thickness, UINT32 color);
VOID gfx_draw_arc(_PGFX_SURFACE surface, INT cx, INT cy, INT radius, INT start_angle, INT end_angle, UINT32 color);
VOID gfx_fill_sector(_PGFX_SURFACE surface, INT cx, INT cy, INT radius, INT start_angle, INT end_angle, UINT32 color);
VOID gfx_fill_triangle(_PGFX_SURFACE surface, INT x0, INT y0, INT x1, INT y1, INT x2, INT y2, UINT32 color);
VOID gfx_draw_line_aa(_PGFX_SURFACE surface, INT x0, INT y0, INT x1, INT y1, UINT32 color);
VOID gfx_clear(_PGFX_SURFACE surface, UINT32 color);
VOID gfx_put_pixel(_PGFX_SURFACE surface, UINT32 x, UINT32 y, UINT32 color);
VOID gfx_fill_rect(_PGFX_SURFACE surface, UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color);
VOID gfx_draw_rect(_PGFX_SURFACE surface, UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color);
VOID gfx_draw_line(_PGFX_SURFACE surface, INT x0, INT y0, INT x1, INT y1, UINT32 color);
VOID gfx_hline(_PGFX_SURFACE surface, INT x, INT y, INT w, UINT32 color);
VOID gfx_vline(_PGFX_SURFACE surface, INT x, INT y, INT h, UINT32 color);
VOID gfx_draw_circle(_PGFX_SURFACE surface, INT cx, INT cy, INT radius, UINT32 color);
VOID gfx_draw_char(_PGFX_SURFACE surface, UINT32 x, UINT32 y, char c, UINT32 color);
VOID gfx_draw_string(_PGFX_SURFACE surface, UINT32 x, UINT32 y, PCCHAR str, UINT32 color);
INT gfx_get_string_width(PCCHAR str);
 
VOID gfx_panel(_PGFX_SURFACE surface, INT x, INT y, INT w, INT h, UINT32 bg);
VOID gfx_bevel_in(_PGFX_SURFACE surface, INT x, INT y, INT w, INT h);
VOID gfx_bevel_out(_PGFX_SURFACE surface, INT x, INT y, INT w, INT h);
VOID gfx_title_bar(_PGFX_SURFACE surface, INT x, INT y, INT w, PCCHAR title);
VOID gfx_button(_PGFX_SURFACE surface, INT x, INT y, INT w, INT h, PCCHAR label, INT pressed);
INT ui_button(_PGFX_SURFACE surface, INT x, INT y, INT w, INT h, PCCHAR label);
VOID gfx_progress_bar(_PGFX_SURFACE surface, INT x, INT y, INT w, INT h, INT percent, UINT32 fill, UINT32 empty);
VOID gfx_list(_PGFX_SURFACE surface, INT x, INT y, INT w, INT h, PCCHAR *items, INT count, INT selected);
VOID gfx_desktop(_PGFX_SURFACE surface);
VOID gfx_status_bar(_PGFX_SURFACE surface, INT x, INT y, INT w, PCCHAR text);
VOID gfx_logo_design2(_PGFX_SURFACE surface, INT x, INT y);
VOID gfx_logo_os(_PGFX_SURFACE surface, INT x, INT y);
VOID gfx_logo_phonon(_PGFX_SURFACE surface, INT x, INT y);
VOID gfx_logo_shadow(_PGFX_SURFACE surface, INT x, INT y);
 
VOID fb_fill_circle(INT cx, INT cy, INT radius, UINT32 color);
VOID fb_draw_line_thick(INT x0, INT y0, INT x1, INT y1, INT thickness, UINT32 color);
VOID fb_draw_vector(INT x0, INT y0, INT angle, INT magnitude, INT thickness, UINT32 color);
VOID fb_draw_arc(INT cx, INT cy, INT radius, INT start_angle, INT end_angle, UINT32 color);
VOID fb_fill_sector(INT cx, INT cy, INT radius, INT start_angle, INT end_angle, UINT32 color);
VOID fb_fill_triangle(INT x0, INT y0, INT x1, INT y1, INT x2, INT y2, UINT32 color);
VOID fb_draw_line_aa(INT x0, INT y0, INT x1, INT y1, UINT32 color);
VOID fb_clear(UINT32 color);
VOID fb_put_pixel(UINT32 x, UINT32 y, UINT32 color);
VOID fb_fill_rect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color);
VOID fb_draw_line(INT x0, INT y0, INT x1, INT y1, UINT32 color);
VOID fb_draw_circle(INT cx, INT cy, INT radius, UINT32 color);
VOID fb_draw_rect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, UINT32 color); /* outline */
VOID fb_draw_char(UINT32 x, UINT32 y, char c, UINT32 color);
VOID fb_draw_string(UINT32 x, UINT32 y, PCCHAR str, UINT32 color);
INT point_in_rect(INT px, INT py, INT x, INT y, INT w, INT h);

#endif /* __GFX_FB_H__ */