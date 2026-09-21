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

#endif /* __GFX_FB_H__ */