$Lattice = "$HOME/OwlyNest/Lattice"
$Edk2    = "$Lattice/edk2"
$Phonon  = "$Lattice/Phonon"

$env:WORKSPACE      = $Phonon
$env:EDK_TOOLS_PATH = "$Edk2/BaseTools"
$env:PACKAGES_PATH  = "$Phonon$([System.IO.Path]::PathSeparator)$Edk2"
$env:PATH           = "$Edk2/BaseTools/BinWrappers/PosixLike$([System.IO.Path]::PathSeparator)$env:PATH"