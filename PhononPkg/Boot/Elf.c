/*
 * Boot/Elf.c - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:56:57 2026
 * Copyright © 2026 OwlyNest
 */

/* --- Styling Instructions ---
 * Encoding:                      UTF-8, Unix line endings
 * Text font:                     Monospace
 * Line width:                    Max 80 characters
 * Indentation:                   Use 4 spaces
 * Brace style:                   Same line as control statement
 * Inline comments:               Column 40, wherever possible, else, whole
 * multiple of 20 Section headers:               Use 3 '-' characters before and
 * after Pointer notation:              Next to variable name, not type Binary
 * operations:             Space around operator Empty parameter list: Use
 * (void) instead of () Statements and declarations:   Max one per line
 */

/* --- Macros ---*/
#define ELF_MAGIC   0x464C457FU // "\x7FELF", little-endian
#define ELFCLASS64  2
#define ET_EXEC     2
#define PT_LOAD     1

/* --- Includes ---*/
#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Elf.h>
/* --- Typedefs - Structs - Enums ---*/
#pragma pack(push, 1)
typedef struct {
  UINT8  e_ident[16];
  UINT16 e_type;
  UINT16 e_machine;
  UINT32 e_version;
  UINT64 e_entry;
  UINT64 e_phoff;
  UINT64 e_shoff;
  UINT32 e_flags;
  UINT16 e_ehsize;
  UINT16 e_phentsize;
  UINT16 e_phnum;
  UINT16 e_shentsize;
  UINT16 e_shnum;
  UINT16 e_shstrndx;
} ELF64_EHDR;
 
typedef struct {
  UINT32 p_type;
  UINT32 p_flags;
  UINT64 p_offset;
  UINT64 p_vaddr;
  UINT64 p_paddr;
  UINT64 p_filesz;
  UINT64 p_memsz;
  UINT64 p_align;
} ELF64_PHDR;
#pragma pack(pop)

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/
EFI_STATUS ElfLoadImage(
    IN VOID   *FileBuffer,
    IN UINTN   FileSize,
    OUT VOID **EntryPoint
) {
    ELF64_EHDR *Ehdr;
    ELF64_PHDR *Phdr;
    UINTN       Index;
    EFI_STATUS  Status;

    if (FileSize < sizeof(ELF64_EHDR)) {
        Print(L"[!] File too small to be an ELF\n");
        return EFI_LOAD_ERROR;
    }

    Ehdr = (ELF64_EHDR *)FileBuffer;

    if (*(UINT32 *)Ehdr->e_ident != ELF_MAGIC) {
        Print(L"[!] Missing ELF magic\n");
        return EFI_LOAD_ERROR;
    }

    if (Ehdr->e_ident[4] != ELFCLASS64) {
        Print(L"[!] Not a 64-bit ELF\n");
        return EFI_LOAD_ERROR;
    }

    if (Ehdr->e_type != ET_EXEC) {
            Print(L"[!] Only static (non-PIE) executables are supported right now\n");
        return EFI_LOAD_ERROR;
    }

    Phdr = (ELF64_PHDR *)((UINT8 *)FileBuffer + Ehdr->e_phoff);

    for (Index = 0; Index < Ehdr->e_phnum; Index++, Phdr++) {
        EFI_PHYSICAL_ADDRESS SegmentAddr;
        UINTN                SegmentPages;

        if (Phdr->p_type != PT_LOAD) {
            continue;
        }

        // Loaded at its physical address: firmware's page tables are still
        // identity-mapped right now, so physical == virtual for the moment.
        // This assumption stops holding the day Shadow moves to a higher-half
        // layout — worth revisiting then, not now.

        SegmentAddr = Phdr->p_paddr;
        SegmentPages = EFI_SIZE_TO_PAGES(Phdr->p_memsz);

        Status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, SegmentPages, &SegmentAddr);
        if (EFI_ERROR(Status)) {
            Print(L"[!] AllocatePages(0x%lx) failed: %r\n", Phdr->p_paddr, Status);
            return Status;
        }

        gBS->CopyMem((VOID *)(UINTN)Phdr->p_paddr, (UINT8 *)FileBuffer + Phdr->p_offset, Phdr->p_filesz);
        if (Phdr->p_memsz > Phdr->p_filesz) {
            // .bss: reserved by p_memsz but not backed by file bytes
            gBS->SetMem(
                (VOID *)((UINTN)Phdr->p_paddr + Phdr->p_filesz),
                Phdr->p_memsz - Phdr->p_filesz,
                0
            );
        }
    }

    *EntryPoint = (VOID *)(UINTN)Ehdr->e_entry;

    return EFI_SUCCESS;
}