/*
 * Shadow/Kernel.c - minimal kernel entry point
 *
 * Reads PhononBootInfo directly (SysV AMD64: arrives in %rdi, which the
 * compiler binds to this parameter automatically) rather than assuming
 * hardcoded struct offsets — the whole point of sharing info.h between
 * the bootloader and Shadow is that the compiler enforces agreement on
 * layout, not that both sides have to independently get it right.
 */

#include <stdint.h>

#include "info.h"

void _start(PhononBootInfo *Info) {
  if (Info->magic != PHONON_BOOT_INFO_MAGIC) {
    // Can't trust anything else in Info if this doesn't match — no
    // framebuffer to draw to safely, nothing to do but halt.
    for (;;) {
      __asm__ __volatile__("cli\n\thlt");
    }
  }

  // Solid red, deliberately different from the bootloader's own green
  // test — proof this is Shadow drawing, using values it read out of
  // Info itself, not a leftover from the bootloader stage.
  uint32_t *framebuffer = (uint32_t *)(uintptr_t)Info->framebuffer_base;
  uint32_t pixels_per_row = Info->framebuffer_pitch / 4;
  uint32_t total_pixels = pixels_per_row * Info->framebuffer_height;

  for (uint32_t i = 0; i < total_pixels; i++) {
    framebuffer[i] = 0x00FF0000;
  }

  for (;;) {
    __asm__ __volatile__("cli\n\thlt");
  }
}