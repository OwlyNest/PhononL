#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <info.h>

#include <Elf.h>
#include <FileIO.h>
#include <Graphics.h>
#include <Acpi.h>
#include <Handoff.h>
#include <Memory.h>

EFI_STATUS EFIAPI UefiMain(
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
) {

    EFI_STATUS Status;
    VOID *FileBuffer;
    UINTN FileSize;
    VOID *KernelEntry;
    PhononBootInfo Info = {0};

    Info.magic = PHONON_BOOT_INFO_MAGIC;
    Info.version = PHONON_BOOT_INFO_VERSION;

    Print(L"Phonon bootloader\n");
    Print(L"Shadow is waiting beyond the lattice.\n");

        Status = GraphicsInit(&Info);
    if (EFI_ERROR(Status)) {
        Print(L"[!] Failed to init graphics: %r\n", Status);
        goto Halt;
    }

    Status = AcpiFindRsdp(&Info);
    if (EFI_ERROR(Status)) {
        Print(L"[!] Failed to find ACPI RSDP: %r\n", Status);
        goto Halt;
    }

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

    Print(L"[x] Kernel entry point: 0x%lx\n", (UINT64)(UINTN)KernelEntry);

    Status = MemoryExitBootServices(ImageHandle, &Info);
    if (EFI_ERROR(Status)) {
        /*
         * MemoryExitBootServices only ever returns success AFTER
         * ExitBootServices has actually succeeded, so on failure path
         * Boot services are still allive. Print/Stall below are safe
        */
        Print(L"[!] Failed to exit boot services: %r\n", Status);
        goto Halt;
    }

    /*
     * Boot services are gone from here on. No gBS, Print or Stall.
     * HandoffJump does not return.
    */
    HandoffJump(KernelEntry, &Info);

Halt:
    while (TRUE) {
        gBS->Stall(1000000);
    }

    return EFI_SUCCESS;
}