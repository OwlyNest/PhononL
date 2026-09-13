$ErrorActionPreference = "Stop"
$ErrorView = 'ConciseView'

$VerboseBuild = $false
$Rebuild      = $true
$Full         = $true
$Arch         = "X64"
$Toolchain    = "GCC"
#$date         = Get-Date -Format yyyy-MM-dd

# Bring in WORKSPACE / PACKAGES_PATH / EDK_TOOLS_PATH / PATH
. "$PSScriptRoot/setup.ps1"

$UsbDevice = $null

for ($i = 0; $i -lt $args.Length; $i++) {
    switch ($args[$i]) {
        "--verbose"    { $VerboseBuild = $true }
        "-v"           { $VerboseBuild = $true }

        "--no-rebuild" { $Rebuild = $false }
        "-nr"          { $Rebuild = $false }

        "--partial"    { $Full = $false }
        "-p"           { $Full = $false }

        # Takes the next argument as the target block device, e.g.
        # ./boo.ps1 --usb /dev/sdb
        "--usb"        { $i++; $UsbDevice = $args[$i] }
    }
}

function Run {
    param(
        [string]$CommandLine
    )

    $parts = $CommandLine -split " "

    $Command   = $parts[0]
    $Arguments = $parts[1..($parts.Length - 1)]

    if ($VerboseBuild) {
        & $Command @Arguments
    }
    else {
        & $Command @Arguments *> $null
    }

    if ($LASTEXITCODE -ne 0) {
        throw "Command failed: $Command (exit code $LASTEXITCODE)"
    }
}

function Get-CandidateDrives {
    # Whole disks only (-d), not partitions — RM marks removable media,
    # which is what a USB stick should show up as.
    Write-Host ""
    Write-Host "Available block devices:"
    & lsblk -dpno NAME,SIZE,RM,MODEL |
        ForEach-Object { Write-Host "  $_" }
    Write-Host ""
}

function Invoke-FlashUsb {
    param([string]$Device)

    if ([string]::IsNullOrWhiteSpace($Device)) {
        Write-Host "[x] No device given, skipping flash."
        return
    }

    if (!(Test-Path $Device)) {
        throw "Device '$Device' does not exist."
    }

    # Refuse to flash whatever disk the running system's root actually
    # lives on — a typo here (sda vs sdb) is exactly the kind of mistake
    # this check exists for.
    $RootSource = & findmnt -no SOURCE / 2>$null
    if ($RootSource -and $RootSource.StartsWith($Device)) {
        throw "Refusing to flash '$Device' — it appears to hold your root filesystem."
    }

    Write-Host ""
    Write-Host "[!] About to overwrite ALL data on $Device with phonon.iso." -ForegroundColor Red
    $Confirm = Read-Host "Type the device path again to confirm ($Device)"
    if ($Confirm -ne $Device) {
        Write-Host "[x] Confirmation did not match — aborting flash, no changes made."
        return
    }

    Write-Host "[x] Flashing phonon.iso -> $Device ..."
    & sudo dd if=phonon.iso of=$Device bs=4M status=progress conv=fsync

    if ($LASTEXITCODE -ne 0) {
        throw "dd failed (exit code $LASTEXITCODE)"
    }

    Write-Host "[x] Flash complete."
}

# Don't "optimize" or change this, you WILL break the line count
function Measure-Lines {
    param(
        [string]$Path = ".",
        [string[]]$Extensions = @(".c", ".c3", ".h", ".S"),
        [string[]]$Exclude = @()
    )

    $total = 0

    Get-ChildItem $Path -Recurse -File |
    Where-Object {
        $file = $_

        (!$Extensions -or $Extensions -contains $file.Extension) -and
        !($Exclude | Where-Object {
                $file.FullName -like "*$_*"
            })
    } |
    ForEach-Object {
        $bytes = [System.IO.File]::ReadAllBytes($_.FullName)

        foreach ($byte in $bytes) {
            if ($byte -eq 10) {
                $total++
            }
        }
    }

    return $total
}

Write-Host "[x] Architecture: $Arch"
Write-Host "[x] Toolchain:    $Toolchain"

if ($Rebuild) {

    Write-Host "[x] Counting Source Lines..."

    $lines_total = Measure-Lines `
        -Extensions @(".c", ".c3", ".h", ".S")

    $lines_phonon = Measure-Lines `
        -Extensions @(".c", ".c3", ".h", ".S") `
        -Exclude @("/Build/", "/third_party/acpica/", "/out/")

    if ($Full) {
        Write-Host "[x] Cleaning previous build..."
        if (Test-Path "Build") { Remove-Item "Build" -Recurse -Force }

        Write-Host "[x] Building PhononPkg + generating compile_commands.json..."
        Remove-Item "compile_commands.json" -ErrorAction SilentlyContinue
        Run "bear -- build -a $Arch -t $Toolchain -p PhononPkg/PhononPkg.dsc"
    }
    else {
        Write-Host "[x] Building PhononPkg..."
        Run "build -a $Arch -t $Toolchain -p PhononPkg/PhononPkg.dsc"
    }

    Write-Host "[x] Source lines: $lines_total"
    Write-Host "[x] Phonon lines: $lines_phonon"

    $EfiOut = "Build/Phonon/DEBUG_$Toolchain/$Arch/PhononBoot.efi"
    if (!(Test-Path $EfiOut)) {
        throw "Build reported success but $EfiOut is missing — check the build log."
    }

    Write-Host "[x] Building Shadow..."
    if ($Full) {
        Run "make -C Shadow clean"
    }
    Run "make -C Shadow"

    $ShadowElf = "Shadow/SHADOW.ELF"
    if (!(Test-Path $ShadowElf)) {
        throw "Shadow build reported success but $ShadowElf is missing."
    }

    Write-Host "[x] Preparing ISO directory structure..."

    # Reset the staging directory cleanly
    if (Test-Path "isodir") { Remove-Item "isodir" -Recurse -Force }
    New-Item -ItemType Directory -Force -Path "isodir\EFI\BOOT" | Out-Null

    Copy-Item `
        $EfiOut `
        "isodir\EFI\BOOT\BOOTX64.EFI" `
        -Force

    Copy-Item `
        $ShadowElf `
        "isodir\SHADOW.ELF" `
        -Force

    Write-Host "[x] Building EFI system partition image..."

    # A small FAT image is what actually gets El Torito-booted by firmware;
    # the ISO itself is just a carrier for it. This is the volume
    # FileIoReadFile actually reads from — the outer ISO9660/Joliet tree in
    # isodir/ is not what UEFI mounts as the boot device, so SHADOW.ELF has
    # to be copied in here too, not just into isodir.
    if (Test-Path "esp.img") { Remove-Item "esp.img" -Force }
    Run "dd if=/dev/zero of=esp.img bs=1M count=16"
    Run "mkfs.vfat esp.img"
    Run "mmd -i esp.img ::/EFI ::/EFI/BOOT"
    Run "mcopy -i esp.img isodir/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI"
    Run "mcopy -i esp.img isodir/SHADOW.ELF ::/SHADOW.ELF"
    Copy-Item "esp.img" "isodir\esp.img" -Force

    Write-Host "[x] Creating EFI-bootable ISO via xorriso..."

    xorriso -as mkisofs `
        -R -J `
        -V "PHONON" `
        -o phonon.iso `
        --efi-boot esp.img `
        -efi-boot-part `
        --efi-boot-image `
        --protective-msdos-label `
        isodir

    if ($LASTEXITCODE -ne 0) {
        throw "xorriso failed (exit code $LASTEXITCODE)"
    }

    xorriso -indev phonon.iso -ls /
    xorriso -indev phonon.iso -ls /EFI/BOOT
}


if (Test-Path "phonon.iso") {
    if ($UsbDevice) {
        Invoke-FlashUsb -Device $UsbDevice
    }
    else {
        Get-CandidateDrives
        $Chosen = Read-Host "Device to flash phonon.iso to (leave empty to skip)"
        Invoke-FlashUsb -Device $Chosen
    }
}

Write-Host "[x] Launching QEMU..."

if (!(Test-Path "OVMF_VARS.4m.fd")) {
    Write-Host "[x] Creating project-local UEFI variables..."
    Copy-Item `
        "/usr/share/edk2/x64/OVMF_VARS.4m.fd" `
        "OVMF_VARS.4m.fd" `
        -Force
}

$qemuArgs = @(
    "-cpu", "max",
    "-cdrom", "phonon.iso",
    "-m", "256",
    "-no-reboot",
    "-serial", "stdio",
    "-monitor", "none",
    "-machine", "q35",
    "-drive", "if=pflash,format=raw,readonly=on,file=/usr/share/edk2/x64/OVMF_CODE.4m.fd",
    "-drive", "if=pflash,format=raw,file=OVMF_VARS.4m.fd",
    "-net", "none",
    "-display", "gtk",
    "-rtc", "base=localtime"
)

& qemu-system-x86_64 @qemuArgs

$qemuExit = $LASTEXITCODE
switch ($qemuExit) {
    1 { Write-Host "[x] Guest exited gracefully." }
    0 { Write-Host "[x] You did the ctrl+C didn't you?" }
    default { Write-Host "[x] QEMU exited with code $qemuExit." }
}