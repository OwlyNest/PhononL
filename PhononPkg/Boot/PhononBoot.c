#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <info.h>

#include <Elf.h>
#include <FileIO.h>

EFI_STATUS EFIAPI UefiMain(
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
) {
    EFI_STATUS Status;
    VOID *FileBuffer;
    UINTN FileSize;
    VOID *KernelEntry;

    Print(L"Phonon bootloader\n");
    Print(L"Shadow is waiting beyond the lattice.\n");

    Status = FileIOReadFile(ImageHandle, L"\\SHADOW.ELF", &FileBuffer, &FileSize);
    if (EFI_ERROR(Status)) {
        Print(L"[!] Failed to read kernel: %r\n\n", Status);
        goto Halt;
    }

    Print(L"[x] Read SHADOW.ELF: %lu bytes\n", (UINT64)FileSize);

    Status = ElfLoadImage(FileBuffer, FileSize, &KernelEntry);
    gBS->FreePool(FileBuffer);
    if (EFI_ERROR(Status)) {
        Print(L"[!] Failed to load kernel: %r\n", Status);
        goto Halt;
    }

    Print(L"[x] Kernel entry point: 0x&lx\n", (UINT64)(UINTN)KernelEntry);

    // Next: Graphics (GOP), Acpi (RSDP), Memory (GetMemoryMap +
    // ExitBootServices), then Handoff jumps to KernelEntry with a populated
    // PhononBootInfo.

Halt:
    while (TRUE) {
        gBS->Stall(1000000);
    }

    return EFI_SUCCESS;
}