#ifndef __INFO_H__
#define __INFO_H__

#include <stdint.h>

#define PHONON_BOOT_INFO_MAGIC 0x50484F4E4F4E0001ULL // "PHONON" + version
#define PHONON_BOOT_INFO_VERSION 1

#define CHECK_STATUS(Status, Msg)                                              \
  do {                                                                         \
    if (EFI_ERROR(Status)) {                                                   \
      Print(L"[!] %s failed: %r\n", (Msg), (Status));                          \
      while (TRUE) {                                                           \
        gBS->Stall(1000000);                                                   \
      }                                                                        \
    }                                                                          \
  } while (0)

typedef struct {
  uint64_t magic;
  uint32_t version;
  uint32_t reserved;

  uint64_t framebuffer_base;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_format;

  uint64_t rsdp_address;

  uint64_t memmap_base;
  uint64_t memmap_entry_count;
  uint64_t memmap_entry_size;
} PhononBootInfo, *PPhononBootInfo;

#endif /* __INFO_H__ */