/*
	* Shadow/GFX/FB.C - framebuffer and 2d graphics
	* Author:   amity
	* Date:     Mon Sep 14 12:42:11 2026
	* Copyright © 2026 OwlyNest
*/

/* --- Styling Instructions ---
	* Encoding:                      UTF-8, Unix line endings
	* Text font:                     Monospace
	* Line Width:                    Max 80 characters
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
#define FB_VIRT_BASE 0xDF000000u
/* --- Includes ---*/
#include <GAL/GAL.H>
#include <GFX/GFX.H>
#include <Lib/Lib.H>
#include <MM/MM.H>
// #include <screen/printk.h>

/*
 * Dropped from this file: <drivers/mouse.h>, <hw/svga.h>,
 * <internal/virtmem.h>, <mm/paging.h>, <mm/pmm.h>. Not a GAL-related
 * change — none of those subsystems are ported into the new tree yet.
 * See the paging-check and mouse_refresh_cursor notes below for where
 * each used to matter and what to restore once it's back.
 *
 * NOTE: lib/string.h, mm/heap.h and screen/printk.h aren't ported yet
 * either as of this writing — this file won't actually compile
 * stand-alone until those three exist. Pre-existing gap, not introduced
 * by this change.
*/

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/
_FB_SURFACE fb;
static INT FBBackOwned = 0;


/* Hardware properties of whatever the GAL's active backend is currently
 * driving — generic, populated from gal_mode_t. fb.c never asks the GAL
 * which backend is active or touches one directly; gal_* calls handle
 * that entirely. */
static struct {
  UINT32 Width;
  UINT32 Height;
  UINT32 Pitch;
  UINT32 Bpp;
  UINT32 RedMask;
  UINT32 GreenMask;
  UINT32 BlueMask;
} FbHw;

/* --- Prototypes ---*/

/* --- Functions ---*/

/* ==========================================================================
 * Initialize Framebuffer
 * ======================================================================= */
int FbInit(void) {
  /* TODO: bring this back once mm/paging.c is ported. Framebuffer
   * addresses from any backend are meaningless until paging is real —
   * this used to guard fb_init() from being called too early. */
  /* if (!paging_enabled()) return -1; */
 
  if (FBBackOwned && fb.Back.Pixels) {
    // free(fb.Back.Pixels);
    fb.Back.Pixels = NULL;
  }
  FBBackOwned = 0;
 
  if (GALInit() != 0) {
    printk("[fb] GAL backend '%s' failed to initialize\r\n", GALBackendName());
    return -1;
  }
 
  _GAL_MODE Mode;
  GALGetMode(&Mode);
 
  FbHw.Width      = Mode.Width;
  FbHw.Height     = Mode.Height;
  FbHw.Pitch      = Mode.Pitch;
  FbHw.Bpp        = Mode.Bpp;
  FbHw.RedMask   = Mode.RedMask;
  FbHw.GreenMask = Mode.GreenMask;
  FbHw.BlueMask  = Mode.BlueMask;
 
  fb.Front = (UINT32 *)GALGetFramebuffer();
  if (!fb.Front) {
    printk("[fb] GAL backend '%s' has no linear framebuffer\r\n", GALBackendName());
    return -1;
  }
 
  /* --- Backbuffer --- */
  fb.Back.Width = FbHw.Width;
  fb.Back.Height = FbHw.Height;
 
/* remove the giant static array completely */

    SIZE_T BufSize = (SIZE_T)fb.Back.Width * fb.Back.Height * sizeof(UINT32);

    fb.Back.Pixels = NULL;
    FBBackOwned    = 0;

    fb.Back.Pixels = ExPoolReady() ? (UINT32 *)ExAllocatePool(BufSize) : NULL;

    if (fb.Back.Pixels) {
        fb.Back.Pitch   = fb.Back.Width * sizeof(UINT32);
        fb.Back.PitchPx = fb.Back.Width;
        FBBackOwned     = 1;
        MemSet(fb.Back.Pixels, 0, BufSize);
    } else {
        fb.Back.Pixels  = fb.Front;
        fb.Back.Pitch   = FbHw.Pitch;
        fb.Back.PitchPx = FbHw.Pitch / (FbHw.Bpp / 8);
    }

  fb.Back.Pitch   = fb.Back.Width * sizeof(UINT32);
  fb.Back.PitchPx = fb.Back.Width;
 
  fb.Initialized = 1;
  printk("[fb] Using GAL backend '%s': %ux%u\r\n", GALBackendName(), FbHw.Width, FbHw.Height);
  return 0;
}

VOID FbUpdateHw(VOID) {
  _GAL_MODE Mode;
  GALGetMode(&Mode);
 
  FbHw.Width      = Mode.Width;
  FbHw.Height     = Mode.Height;
  FbHw.Pitch      = Mode.Pitch;
  FbHw.Bpp        = Mode.Bpp;
  FbHw.RedMask    = Mode.RedMask;
  FbHw.GreenMask  = Mode.GreenMask;
  FbHw.BlueMask   = Mode.BlueMask;
 
  fb.Front = (UINT32 *)GALGetFramebuffer();
}

/* ==========================================================================
 * Present backbuffer to screen
 * ======================================================================= */
VOID FbPresent(VOID) {
  if (!fb.Initialized)
    return;
 
  if (FbHw.Bpp == 32 && fb.Back.Pitch == FbHw.Pitch) {
    MemCpy(fb.Front, fb.Back.Pixels, fb.Back.Pitch * fb.Back.Height);
  } else {
    for (uint32_t y = 0; y < fb.Back.Height; y++) {
      uint32_t *src_row = fb.Back.Pixels + y * fb.Back.PitchPx;
      uint8_t *dst_row = (uint8_t *)fb.Front + y * FbHw.Pitch;
 
      for (uint32_t x = 0; x < fb.Back.Width; x++) {
        uint32_t color = src_row[x];
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
 
        if (FbHw.Bpp == 24) {
          dst_row[x * 3 + 0] = b;
          dst_row[x * 3 + 1] = g;
          dst_row[x * 3 + 2] = r;
        } else if (FbHw.Bpp == 16) {
          uint16_t *dst16 = (uint16_t *)dst_row;
          dst16[x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        }
      }
    }
  }
 
  /* Backend-specific extras (a GPU FIFO kick, etc). A no-op for a plain
   * linear framebuffer like GOP's — SVGA-II's svga_update_full() call
   * moves into ITS OWN present() once that backend is ported. */
  GALPresent(&fb.Back);
 
  /* mouse_refresh_cursor() removed for now — drivers/mouse.c isn't
   * ported yet. Bring it back once it is; it never had anything to do
   * with which backend is active, so it stays right here either way. */
}


UINT32 FbPackPixel(UINT8 r, UINT8 g, UINT8 b) {
  if (FbHw.Bpp == 32) {
    if (FbHw.RedMask == 0x00FF0000) {
      return ((UINT32)r << 16) | ((UINT32)g << 8) | b;
    } else if (FbHw.RedMask == 0x000000FF) {
      return ((UINT32)b << 16) | ((UINT32)g << 8) | r;
    }
    return ((UINT32)r << 16) | ((UINT32)g << 8) | b;
  }
  if (FbHw.Bpp == 24) {
    return ((UINT32)r << 16) | ((UINT32)g << 8) | b;
  }
  if (FbHw.Bpp == 16) {
    UINT32 r5 = (r >> 3) & 0x1F;
    UINT32 g6 = (g >> 2) & 0x3F;
    UINT32 b5 = (b >> 3) & 0x1F;
    return (r5 << 11) | (g6 << 5) | b5;
  }
  if (FbHw.Bpp == 15) {
    UINT32 r5 = (r >> 3) & 0x1F;
    UINT32 g5 = (g >> 3) & 0x1F;
    UINT32 b5 = (b >> 3) & 0x1F;
    return (r5 << 10) | (g5 << 5) | b5;
  }
  return ((UINT32)r << 16) | ((UINT32)g << 8) | b;
}

/* ==========================================================================
 * Color helpers
 * ======================================================================= */
UINT32 GfxThemeColor(_GFX_THEME_COLOR c) {
  switch (c) {
  case GFX_BG_DESKTOP:
    return FbPackPixel(20, 30, 100);
  case GFX_BG_PANEL:
    return FbPackPixel(40, 45, 60);
  case GFX_BG_TITLE:
    return FbPackPixel(80, 90, 120);
  case GFX_BG_HIGHLIGHT:
    return FbPackPixel(100, 120, 160);
  case GFX_BG_BUTTON:
    return FbPackPixel(60, 70, 90);
  case GFX_BG_BUTTON_HOVER:
    return FbPackPixel(80, 95, 120);
  case GFX_FG_TEXT:
    return FbPackPixel(255, 255, 255);
  case GFX_FG_TEXT_DIM:
    return FbPackPixel(180, 180, 200);
  case GFX_FG_ACCENT:
    return FbPackPixel(100, 200, 255);
  case GFX_BORDER_LIGHT:
    return FbPackPixel(120, 130, 150);
  case GFX_BORDER_DARK:
    return FbPackPixel(20, 25, 35);
  case GFX_RED:
    return FbPackPixel(255, 0, 0);
  case GFX_GREEN:
    return FbPackPixel(0, 255, 0);
  case GFX_BLUE:
    return FbPackPixel(0, 0, 255);
  case GFX_YELLOW:
    return FbPackPixel(255, 255, 0);
  case GFX_WHITE:
    return FbPackPixel(255, 255, 255);
  case GFX_BLACK:
    return FbPackPixel(0, 0, 0);
  default:
    return FbPackPixel(255, 255, 255);
  }
}

/* ==========================================================================
 * Basic Drawing
 * ======================================================================= */
VOID FbClear(UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxClear(&fb.Back, color);
}

VOID GfxClear(_PGFX_SURFACE surface, UINT32 color) {
  for (UINT32 y = 0; y < surface->Height; y++) {
    UINT32 *row = surface->Pixels + y * surface->PitchPx;
    for (UINT32 x = 0; x < surface->Width; x++) {
      row[x] = color;
    }
  }
}

VOID FbPutPixel(UINT32 x, UINT32 y, UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxPutPixel(&fb.Back, x, y, color);
}
VOID GfxPutPixel(_PGFX_SURFACE surface, UINT32 x, UINT32 y, UINT32 color) {
  if (x >= surface->Width || y >= surface->Height)
    return;
  surface->Pixels[y * surface->PitchPx + x] = color;
}

VOID FbFillRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h,
                  UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxFillRect(&fb.Back, x, y, w, h, color);
}

VOID GfxFillRect(_PGFX_SURFACE surface, UINT32 x, UINT32 y, UINT32 w,
                   UINT32 h, UINT32 color) {
  if (x + w > surface->Width)
    w = surface->Width - x;
  if (y + h > surface->Height)
    h = surface->Height - y;
  if (x >= surface->Width || y >= surface->Height)
    return;

  for (UINT32 row = y; row < y + h; row++) {
    UINT32 *dest = surface->Pixels + row * surface->PitchPx + x;
    for (UINT32 col = 0; col < w; col++) {
      dest[col] = color;
    }
  }
}

VOID FbDrawRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h,
                  UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawRect(&fb.Back, x, y, w, h, color);
}
VOID GfxDrawRect(_PGFX_SURFACE surface, UINT32 x, UINT32 y, UINT32 w,
                   UINT32 h, UINT32 color) {
  GfxDrawLine(surface, x, y, x + w, y, color);
  GfxDrawLine(surface, x, y, x, y + h, color);
  GfxDrawLine(surface, x + w, y, x + w, y + h, color);
  GfxDrawLine(surface, x, y + h, x + w, y + h, color);
}

/* ==========================================================================
 * Bresenham's Line Algorithm
 * ======================================================================= */
static inline int abs(int x) { return x < 0 ? -x : x; }

VOID FbDrawLine(int x0, int y0, int x1, int y1, UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawLine(&fb.Back, x0, y0, x1, y1, color);
}

VOID GfxDrawLine(_PGFX_SURFACE surface, int x0, int y0, int x1, int y1,
                   UINT32 color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;

  while (1) {
    GfxPutPixel(surface, x0, y0, color);
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y0 += sy;
    }
  }
}

VOID gfx_hline(_PGFX_SURFACE surface, int x, int y, int w, UINT32 color) {
  int h = 1;
  GfxFillRect(surface, (UINT32)x, (UINT32)y, (UINT32)w, h, color);
}

VOID gfx_vline(_PGFX_SURFACE surface, int x, int y, int h, UINT32 color) {
  int w = 1;
  GfxFillRect(surface, (UINT32)x, (UINT32)y, w, (UINT32)h, color);
}

/* ==========================================================================
 * Circle (midpoint algorithm)
 * ======================================================================= */
VOID FbDrawCircle(int cx, int cy, int radius, UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawCircle(&fb.Back, cx, cy, radius, color);
}

VOID GfxDrawCircle(_PGFX_SURFACE surface, int cx, int cy, int radius,
                     UINT32 color) {
  if (radius <= 0)
    return;

  int x = radius - 1;
  int y = 0;
  int dx = 1;
  int dy = 1;
  int err = dx - (radius << 1);

  while (x >= y) {
    GfxPutPixel(surface, cx + x, cy + y, color);
    GfxPutPixel(surface, cx + y, cy + x, color);
    GfxPutPixel(surface, cx - y, cy + x, color);
    GfxPutPixel(surface, cx - x, cy + y, color);
    GfxPutPixel(surface, cx - x, cy - y, color);
    GfxPutPixel(surface, cx - y, cy - x, color);
    GfxPutPixel(surface, cx + y, cy - x, color);
    GfxPutPixel(surface, cx + x, cy - y, color);

    if (err <= 0) {
      y++;
      err += dy;
      dy += 2;
    }
    if (err > 0) {
      x--;
      dx += 2;
      err += dx - (radius << 1);
    }
  }
}

VOID fb_draw_char(UINT32 x, UINT32 y, char c, UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawChar(&fb.Back, x, y, c, color);
}
VOID GfxDrawChar(_PGFX_SURFACE surface, UINT32 x, UINT32 y, char c,
                   UINT32 color) {
  if (x >= surface->Width || y >= surface->Height)
    return;
  unsigned char uc = (unsigned char)c;
  if (uc < 32 || uc >= 128)
    uc = '?';

  const UINT8 *glyph = font8x8[uc];

  for (int row = 0; row < 8; row++) {
    if (y + row >= surface->Height)
      break;
    UINT8 line = glyph[row];
    for (int col = 0; col < 8; col++) {
      if (x + col >= surface->Width)
        break;
      if (line & (1u << (7 - col))) {
        GfxPutPixel(surface, x + col, y + row, color);
      }
    }
  }
}

VOID fb_draw_string(UINT32 x, UINT32 y, const char *str, UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawString(&fb.Back, x, y, str, color);
}
VOID GfxDrawString(_PGFX_SURFACE surface, UINT32 x, UINT32 y,
                     const char *str, UINT32 color) {
  if (!str)
    return;

  UINT32 cx = x;
  while (*str) {
    GfxDrawChar(surface, cx, y, *str++, color);
    cx += 8;
  }
}

/* ==========================================================================
 * Filled circle (scanline)
 * ======================================================================= */
VOID fb_fill_circle(int cx, int cy, int radius, UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxFillCircle(&fb.Back, cx, cy, radius, color);
}

VOID GfxFillCircle(_PGFX_SURFACE surface, int cx, int cy, int radius,
                     UINT32 color) {
  if (radius <= 0)
    return;

  int r_sq = radius * radius;

  for (int y = -radius; y <= radius; y++) {
    int row = cy + y;
    if (row < 0 || row >= (int)surface->Height)
      continue;

    int x_len = (int)fx_to_int(fx_sqrt(fx_from_int(r_sq - y * y)));
    int x0 = cx - x_len;
    int x1 = cx + x_len;

    if (x0 < 0)
      x0 = 0;
    if (x1 >= (int)surface->Width)
      x1 = surface->Width - 1;

    if (x0 <= x1) {
      UINT32 *dest = surface->Pixels + row * surface->PitchPx + x0;
      for (int x = x0; x <= x1; x++) {
        *dest++ = color;
      }
    }
  }
}

/* ==========================================================================
 * Thick line: draw a line with circular pen of given radius
 * Uses Bresenham + perpendicular fill
 * ======================================================================= */
VOID FbDrawLineThick(int x0, int y0, int x1, int y1, int thickness,
                        UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawLineThick(&fb.Back, x0, y0, x1, y1, thickness, color);
}

VOID GfxDrawLineThick(_PGFX_SURFACE surface, int x0, int y0, int x1, int y1, int thickness, UINT32 color) {
  if (thickness <= 1) {
    GfxDrawLine(surface, x0, y0, x1, y1, color);
    return;
  }

  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  int r = thickness / 2;
  if (r < 1)
    r = 1;

  while (1) {
    /* Draw a filled circle at each pixel of the line */
    GfxFillCircle(surface, x0, y0, r, color);

    if (x0 == x1 && y0 == y1)
      break;

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

/* ==========================================================================
 * Vector: line from (x0,y0) at angle with magnitude
 * Angle: tenths of degrees, 0 = right (3 o'clock), CCW
 * ======================================================================= */
VOID FbDrawVector(int x0, int y0, int angle, int magnitude, int thickness,
                    UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawVector(&fb.Back, x0, y0, angle, magnitude, thickness, color);
}

VOID GfxDrawVector(_PGFX_SURFACE surface, int x0, int y0, int angle, int magnitude, int thickness, UINT32 color) {
  Vec2I end = vec2i_polar_bradians(x0, y0, magnitude, angle);
  GfxDrawLineThick(surface, x0, y0, end.x, end.y, thickness, color);
}

/* ==========================================================================
 * Arc outline: draw arc from start_angle to end_angle (tenths of degrees)
 * ======================================================================= */
VOID fb_draw_arc(int cx, int cy, int radius, int start_angle, int end_angle,
                 UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxDrawArc(&fb.Back, cx, cy, radius, start_angle, end_angle, color);
}

VOID GfxDrawArc(_PGFX_SURFACE surface, int cx, int cy, int radius,
                  int start_angle, int end_angle, UINT32 color) {
  if (radius <= 0)
    return;

  /* Normalize angles */
  start_angle = start_angle % ANGLE_FULL_BRAD;
  if (start_angle < 0)
    start_angle += ANGLE_FULL_BRAD;
  end_angle = end_angle % ANGLE_FULL_BRAD;
  if (end_angle < 0)
    end_angle += ANGLE_FULL_BRAD;

  int step = 10; /* 1 degree steps, adjust for smoothness vs speed */
  if (radius > 100)
    step = 5;
  if (radius > 300)
    step = 2;

  int a = start_angle;
  int done = 0;

  while (!done) {
    Vec2I p = vec2i_polar_bradians(cx, cy, radius, a);
    GfxPutPixel(surface, p.x, p.y, color);

    if (a == end_angle) {
      done = 1;
    } else {
      a += step;
      if (a >= ANGLE_FULL_BRAD)
        a -= ANGLE_FULL_BRAD;
      /* If we would overshoot end_angle, snap to it */
      if (step > 0 &&
          ((end_angle > start_angle && a > end_angle) ||
           (end_angle < start_angle && a > end_angle && a < start_angle))) {
        a = end_angle;
      }
    }
  }
}

/* ==========================================================================
 * Sector fill (pie slice): fill area between two angles
 * ======================================================================= */
VOID fb_fill_sector(int cx, int cy, int radius, int start_angle, int end_angle,
                    UINT32 color) {
  if (!fb.Initialized)
    return;
  GfxFillSector(&fb.Back, cx, cy, radius, start_angle, end_angle, color);
}

VOID GfxFillSector(_PGFX_SURFACE surface, int cx, int cy, int radius,
                     int start_angle, int end_angle, UINT32 color) {
  if (radius <= 0)
    return;

  start_angle = start_angle % ANGLE_FULL_BRAD;
  if (start_angle < 0)
    start_angle += ANGLE_FULL_BRAD;
  end_angle = end_angle % ANGLE_FULL_BRAD;
  if (end_angle < 0)
    end_angle += ANGLE_FULL_BRAD;

  int step = 15; /* 1.5 degree triangles */
  int a = start_angle;

  Vec2I center = vec2i(cx, cy);
  Vec2I prev = vec2i_polar_bradians(cx, cy, radius, a);

  while (1) {
    int next_a = a + step;
    if (next_a > end_angle && a != end_angle) {
      next_a = end_angle;
    } else if (a == end_angle) {
      break;
    }

    Vec2I next = vec2i_polar_bradians(cx, cy, radius, next_a);
    GfxFillTriangle(surface, center.x, center.y, prev.x, prev.y, next.x,
                      next.y, color);

    if (next_a == end_angle)
      break;
    a = next_a;
    prev = next;
  }
}

/* ==========================================================================
 * Triangle fill helpers – integer only (no float / SSE)
 * ======================================================================= */
static VOID gfx_fill_flat_top_triangle(_PGFX_SURFACE surface,
                                       INT x0, INT y0,
                                       INT x1, INT y1,
                                       INT x2, INT y2,
                                       UINT32 color) {
    /* y0 == y1, y2 is the bottom vertex */
    if (y2 == y0)
        return;

    /* 16.16 fixed-point inverse slopes */
    INT32 inv_slope_0 = ((INT32)(x2 - x0) << 16) / (y2 - y0);
    INT32 inv_slope_1 = ((INT32)(x2 - x1) << 16) / (y2 - y1);

    INT32 x_start = (INT32)x0 << 16;
    INT32 x_end   = (INT32)x1 << 16;

    for (INT y = y0; y <= y2; y++) {
        INT x_s = x_start >> 16;
        INT x_e = x_end   >> 16;
        if (x_s > x_e) {
            INT t = x_s;
            x_s = x_e;
            x_e = t;
        }

        if (y >= 0 && y < (INT)surface->Height) {
            if (x_s < 0)
                x_s = 0;
            if (x_e >= (INT)surface->Width)
                x_e = surface->Width - 1;
            UINT32 *dest = surface->Pixels + y * surface->PitchPx + x_s;
            for (INT x = x_s; x <= x_e; x++)
                *dest++ = color;
        }

        x_start += inv_slope_0;
        x_end   += inv_slope_1;
    }
}

static VOID gfx_fill_flat_bottom_triangle(_PGFX_SURFACE surface,
                                          INT x0, INT y0,
                                          INT x1, INT y1,
                                          INT x2, INT y2,
                                          UINT32 color) {
    /* y1 == y2, y0 is the top vertex */
    if (y1 == y0) {
        return;
    }

    INT32 inv_slope_0 = ((INT32)(x1 - x0) << 16) / (y1 - y0);
    INT32 inv_slope_1 = ((INT32)(x2 - x0) << 16) / (y2 - y0);

    INT32 x_start = (INT32)x0 << 16;
    INT32 x_end   = (INT32)x0 << 16;

    for (INT y = y0; y <= y1; y++) {
        INT x_s = x_start >> 16;
        INT x_e = x_end   >> 16;
        if (x_s > x_e) {
            INT t = x_s;
            x_s = x_e;
            x_e = t;
        }

        if (y >= 0 && y < (INT)surface->Height) {
            if (x_s < 0) {
                x_s = 0;
            }
            if (x_e >= (INT)surface->Width) {
                x_e = surface->Width - 1;
            }
            UINT32 *dest = surface->Pixels + y * surface->PitchPx + x_s;
            for (INT x = x_s; x <= x_e; x++) {
                *dest++ = color;
            }
        }

        x_start += inv_slope_0;
        x_end   += inv_slope_1;
    }
}

VOID fb_fill_triangle(int x0, int y0, int x1, int y1, int x2, int y2, UINT32 color) {
  if (!fb.Initialized) {
    return;
  }
  GfxFillTriangle(&fb.Back, x0, y0, x1, y1, x2, y2, color);
}

VOID GfxFillTriangle(_PGFX_SURFACE surface, int x0, int y0, int x1, int y1, int x2, int y2, UINT32 color) {
  /* Sort by Y */
  if (y0 > y1) {
    int t;
    t = x0;
    x0 = x1;
    x1 = t;
    t = y0;
    y0 = y1;
    y1 = t;
  }
  if (y1 > y2) {
    int t;
    t = x1;
    x1 = x2;
    x2 = t;
    t = y1;
    y1 = y2;
    y2 = t;
  }
  if (y0 > y1) {
    int t;
    t = x0;
    x0 = x1;
    x1 = t;
    t = y0;
    y0 = y1;
    y1 = t;
  }

  if (y0 == y2)
    return; /* degenerate */

  if (y1 == y2) {
    /* Flat top */
    gfx_fill_flat_bottom_triangle(surface, x0, y0, x1, y1, x2, y2, color);
  } else if (y0 == y1) {
    /* Flat bottom */
    gfx_fill_flat_top_triangle(surface, x0, y0, x1, y1, x2, y2, color);
    } else {
        /* Split into flat-bottom + flat-top */
        INT dy = y2 - y0;
        INT x3 = x0;
        if (dy != 0) {
            x3 = x0 + ((INT32)(y1 - y0) * (x2 - x0)) / dy;
        }
        INT y3 = y1;

        gfx_fill_flat_bottom_triangle(surface, x0, y0, x1, y1, x3, y3, color);
        gfx_fill_flat_top_triangle(surface, x1, y1, x3, y3, x2, y2, color);
    }
}

int gfx_get_string_Width(const char *str) {
  if (!str)
    return 0;
  int len = 0;
  while (*str++)
    len++;
  return len * 8;
}

/* ==========================================================================
 * Panels and 3D borders
 * ======================================================================= */
VOID gfx_panel(_PGFX_SURFACE surface, int x, int y, int w, int h,
               UINT32 bg) {
  GfxFillRect(surface, x, y, w, h, bg);
}

VOID gfx_bevel_out(_PGFX_SURFACE surface, int x, int y, int w, int h) {
  UINT32 light = GfxThemeColor(GFX_BORDER_LIGHT);
  UINT32 dark = GfxThemeColor(GFX_BORDER_DARK);
  gfx_hline(surface, x, y, w, light);
  gfx_vline(surface, x, y, h, light);
  gfx_hline(surface, x, y + h - 1, w, dark);
  gfx_vline(surface, x + w - 1, y, h, dark);
}

VOID gfx_bevel_in(_PGFX_SURFACE surface, int x, int y, int w, int h) {
  UINT32 light = GfxThemeColor(GFX_BORDER_LIGHT);
  UINT32 dark = GfxThemeColor(GFX_BORDER_DARK);
  gfx_hline(surface, x, y, w, dark);
  gfx_vline(surface, x, y, h, dark);
  gfx_hline(surface, x, y + h - 1, w, light);
  gfx_vline(surface, x + w - 1, y, h, light);
}

/* ==========================================================================
 * Title bar
 * ======================================================================= */
VOID gfx_title_bar(_PGFX_SURFACE surface, int x, int y, int w,
                   const char *title) {
  UINT32 bg = GfxThemeColor(GFX_BG_TITLE);
  UINT32 fg = GfxThemeColor(GFX_FG_TEXT);

  GfxFillRect(surface, x, y, w, 20, bg);
  if (title) {
    GfxDrawString(surface, x + 4, y + 6, title, fg);
  }
  gfx_bevel_out(surface, x, y, w, 20);
}

/* ==========================================================================
 * Button
 * ======================================================================= */
VOID gfx_button(_PGFX_SURFACE surface, int x, int y, int w, int h,
                const char *label, int pressed) {
  UINT32 bg = pressed ? GfxThemeColor(GFX_BG_BUTTON_HOVER)
                        : GfxThemeColor(GFX_BG_BUTTON);
  UINT32 fg = GfxThemeColor(GFX_FG_TEXT);

  GfxFillRect(surface, x, y, w, h, bg);
  if (pressed == 1) {
    gfx_bevel_in(surface, x, y, w, h);
  } else {
    gfx_bevel_out(surface, x, y, w, h);
  }

  if (label) {
    int tw = gfx_get_string_Width(label);
    int tx = x + (w - tw) / 2;
    int ty = y + (h - 8) / 2;
    GfxDrawString(surface, tx, ty, label, fg);
  }
}

/* ==========================================================================
 * Progress bar
 * ======================================================================= */
VOID gfx_progress_bar(_PGFX_SURFACE surface, int x, int y, int w, int h,
                      int percent, UINT32 fill, UINT32 empty) {
  GfxFillRect(surface, x, y, w, h, empty);
  gfx_bevel_in(surface, x, y, w, h);

  int fill_w = (w - 4) * percent / 100;
  if (fill_w > 0) {
    GfxFillRect(surface, x + 2, y + 2, fill_w, h - 4, fill);
  }
}

/* ==========================================================================
 * List box
 * ======================================================================= */
VOID gfx_list(_PGFX_SURFACE surface, int x, int y, int w, int h,
              const char **items, int count, int selected) {
  UINT32 bg = GfxThemeColor(GFX_BG_PANEL);
  UINT32 fg = GfxThemeColor(GFX_FG_TEXT);
  UINT32 hi = GfxThemeColor(GFX_BG_HIGHLIGHT);
  UINT32 hifg = GfxThemeColor(GFX_FG_ACCENT);

  GfxFillRect(surface, x, y, w, h, bg);
  gfx_bevel_in(surface, x, y, w, h);

  int content_x = x + 4;
  int content_y = y + 4;
  int content_w = w - 8;
  int content_h = h - 8;

  int row_h = 20;
  int visible = content_h / row_h;
  int start = 0;
  if (selected >= visible) {
    start = selected - visible + 1;
  }

  for (int i = 0; i < visible && (start + i) < count; i++) {
    int idx = start + i;
    int row_y = content_y + i * row_h;
    UINT32 row_bg = (idx == selected) ? hi : bg;
    UINT32 row_fg = (idx == selected) ? hifg : fg;

    GfxFillRect(surface, content_x, row_y, content_w, row_h, row_bg);
    GfxDrawString(surface, content_x + 4, row_y + 6, items[idx], row_fg);
  }
}

/* ==========================================================================
 * Status / task bar
 * ======================================================================= */
VOID gfx_status_bar(_PGFX_SURFACE surface, int x, int y, int w,
                    const char *text) {
  UINT32 bg = GfxThemeColor(GFX_BG_TITLE);
  UINT32 fg = GfxThemeColor(GFX_FG_TEXT_DIM);

  GfxFillRect(surface, x, y, w, 24, bg);
  gfx_bevel_out(surface, x, y, w, 24);
  if (text) {
    GfxDrawString(surface, x + 4, y + 8, text, fg);
  }
}

/* ==========================================================================
 * Desktop background
 * ======================================================================= */
VOID GfxDesktop(_PGFX_SURFACE surface) {
  GfxClear(surface, GfxThemeColor(GFX_BG_DESKTOP));
}

/* ==========================================================================
 * Design 2
 * ======================================================================= */
VOID GfxLogoDesign2(_PGFX_SURFACE surface, int x, int y) {
  GfxFillRect(surface, x, y, 150, 150, GfxThemeColor(GFX_RED));
  GfxFillRect(surface, x + 25, y + 25, 150, 150, GfxThemeColor(GFX_GREEN));
  GfxFillRect(surface, x + 50, y + 50, 150, 150, GfxThemeColor(GFX_BLUE));
  GfxFillRect(surface, x + 50, y + 50, 125, 125, GfxThemeColor(GFX_RED));
  GfxDrawString(surface, x, y - 10, "Welcome to AmitX!",
                  GfxThemeColor(GFX_WHITE));
}

VOID GfxLogoOs(_PGFX_SURFACE surface, int x, int y) {
  GfxLogoPhonon(surface, x, y);
  GfxLogoShadow(surface, x + 160, y);
}

VOID GfxLogoPhonon(_PGFX_SURFACE surface, int x, int y) {
  GfxFillRect(surface, x + 5, y + 5, 70, 70, FbPackPixel(0xF7, 0xA8, 0xB8));
  GfxFillRect(surface, x + 80, y, 70, 70, GfxThemeColor(GFX_GREEN));
  GfxFillRect(surface, x, y + 80, 70, 70, GfxThemeColor(GFX_BLUE));
  GfxFillRect(surface, x + 75, y + 75, 70, 70, GfxThemeColor(GFX_RED));
  GfxDrawString(surface, x + 10, y - 10, "Welcome to Phonon!",
                  GfxThemeColor(GFX_WHITE));
}

VOID GfxLogoShadow(_PGFX_SURFACE surface, int x, int y) {
  GfxDrawRect(surface, x, y, 70, 70, FbPackPixel(0xF7, 0xA8, 0xB8));
  GfxDrawRect(surface, x + 75, y + 5, 70, 70, GfxThemeColor(GFX_GREEN));
  GfxDrawRect(surface, x + 5, y + 75, 70, 70, GfxThemeColor(GFX_BLUE));
  GfxFillRect(surface, x + 80, y + 80, 70, 70, GfxThemeColor(GFX_RED));
  GfxDrawString(surface, x, y - 10, "Powered by Shadow!",
                  GfxThemeColor(GFX_WHITE));
}

int point_in_rect(int px, int py, int x, int y, int w, int h) {
  return px >= x && px < x + w && py >= y && py < y + h;
}