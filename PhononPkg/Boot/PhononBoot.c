#include "Protocol/GraphicsOutput.h"
#include "Uefi/UefiBaseType.h"
#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

EFI_STATUS EFIAPI UefiMain(IN EFI_HANDLE ImageHandle,
                           IN EFI_SYSTEM_TABLE *SystemTable) {
  Print(L"Phonon\n");
  Print(L"Shadow is waiting beyond the lattice.\n");

  while (TRUE) {
    gBS->Stall(1000000);
  }

  return EFI_SUCCESS;
}