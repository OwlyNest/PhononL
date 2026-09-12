/*
 * Boot/FileIO.c - [Enter description]
 * Author:   amity
 * Date:     Thu Sep 10 14:46:05 2026
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

/* --- Includes ---*/
#include <Uefi.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

#include <FileIO.h>

/* --- Typedefs - Structs - Enums ---*/

/* --- Globals ---*/

/* --- Prototypes ---*/

/* --- Functions ---*/
EFI_STATUS FileIOReadFile(
    IN EFI_HANDLE ImageHandle,
    IN CHAR16 *Path,
    OUT void **Buffer,
    OUT UINTN *BufferSize
) {
    EFI_STATUS                       Status;
    EFI_LOADED_IMAGE_PROTOCOL       *LoadedImage;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    EFI_FILE_PROTOCOL               *Root;
    EFI_FILE_PROTOCOL               *File;
    EFI_FILE_INFO                   *FileInfo;
    UINTN                            FileInfoSize;
    VOID                            *FileBuffer;
    UINTN                            FileSize;

    Status = gBS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage);
    if (EFI_ERROR(Status)) {
        Print(L"[!] HandleProtocol(LoadedImage) failed: %r\n", Status);
        return Status;
    }

    Status = gBS->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&FileSystem);
    if (EFI_ERROR(Status)) {
        Print(L"[!] HandleProtocol(SimpleFileSystem) failed: %r\n", Status);
        return Status;
    }

    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status)) {
        Print(L"[!] OpenVolume failed: %r\n", Status);
        return Status;
    }

    Status = Root->Open(Root, &File, Path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        Print(L"[!] Open(%s) failed: %r\n", Path, Status);
        Root->Close(Root);
        return Status;
    }

    // First call is intentionally undersized just to learn the required size
    FileInfoSize = 0;
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileInfoSize, NULL);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        Print(L"[!] GetInfo(size) failed: %r\n", Status);
        goto CloseAndReturn;
    }

    Status = gBS->AllocatePool(EfiLoaderData, FileInfoSize, (VOID **)&FileInfo);
    if (EFI_ERROR(Status)) {
        goto CloseAndReturn;
    }

    Status = File->GetInfo(File, &gEfiFileInfoGuid, &FileSize, FileInfo);
    if (EFI_ERROR(Status)) {
        Print(L"[!] GetInfo failed: %r\n", Status);
        gBS->FreePool(FileInfo);
        goto CloseAndReturn;
    }

    FileSize = (UINTN)FileInfo->FileSize;
    gBS->FreePool(FileInfo);

    Status = gBS->AllocatePool(EfiLoaderData, FileSize, &FileBuffer);
    if (EFI_ERROR(Status)) {
        goto CloseAndReturn;
    }

    Status = File->Read(File, &FileSize, FileBuffer);
    if (EFI_ERROR(Status)) {
        Print(L"[!] Read failed: %r\n", Status);
        gBS->FreePool(FileBuffer);
    }

    *Buffer     = FileBuffer;
    *BufferSize = FileSize;


CloseAndReturn:
    File->Close(File);
    Root->Close(Root);
    return Status;
}