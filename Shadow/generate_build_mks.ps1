$ErrorActionPreference = "Stop"

$root = $PSScriptRoot

function Get-RelativePath {
    param([string]$Path)

    return [System.IO.Path]::GetRelativePath(
        $root,
        [System.IO.Path]::GetFullPath($Path)
    ).Replace("\", "/")
}

$dirs = @(
    "XAL",
    "GFX",
    "GAL",
    "IAL",
    "Lib",
    "MM",
    "Arch\X64"
)

foreach ($dir in $dirs) {
    $fullDir = Join-Path $root $dir
    $mk = Join-Path $fullDir "build.mk"

    if (Test-Path $mk) {
        Remove-Item $mk
    }

    $lines = @(
        "# Auto-generated build.mk for $dir",
        ""
    )

    $cFiles = Get-ChildItem $fullDir -Filter "*.C" -File -ErrorAction SilentlyContinue
    foreach ($file in $cFiles) {
        $lines += "C_SRCS += $(Get-RelativePath $file.FullName)"
    }

    $cppFiles = Get-ChildItem $fullDir -Filter "*.CPP" -File -ErrorAction SilentlyContinue
    foreach ($file in $cppFiles) {
        $lines += "CPP_SRCS += $(Get-RelativePath $file.FullName)"
    }

    $asmFiles = Get-ChildItem $fullDir -Filter "*.ASM" -File -ErrorAction SilentlyContinue
    foreach ($file in $asmFiles) {
        $lines += "ASM_SRCS += $(Get-RelativePath $file.FullName)"
    }

    $asmFiles = Get-ChildItem $fullDir -Filter "*.S" -File -ErrorAction SilentlyContinue
    foreach ($file in $asmFiles) {
        $lines += "GAS_SRCS += $(Get-RelativePath $file.FullName)"
    }

    $rustFiles = Get-ChildItem $fullDir -Filter "*.rs" -File -ErrorAction SilentlyContinue
    foreach ($file in $rustFiles) {
        $lines += "RUST_SRCS += $(Get-RelativePath $file.FullName)"
    }

    $c3Files = Get-ChildItem $fullDir -Filter "*.c3" -File -ErrorAction SilentlyContinue
    foreach ($file in $c3Files) {
        $lines += "C3_SRCS += $(Get-RelativePath $file.FullName)"
    }

    $lines | Set-Content $mk -Encoding utf8
    Write-Host "Generated $dir\build.mk"
}

# ACPICA (unchanged)
# $acpica = Join-Path $root "third_party\acpica\components"
# if (-not (Test-Path $acpica)) {
#     throw "ACPICA directory not found: $acpica"
# }

# $lines = @(
#     "# Auto-generated ACPICA sources",
#     ""
# )

# $count = 0
# try {
#     [System.IO.Directory]::EnumerateFiles(
#         $acpica,
#         "*.c",
#         [System.IO.SearchOption]::AllDirectories
#     ) | ForEach-Object {
#         $count++
#         $path = Get-RelativePath $_
#         $lines += "ACPICA_SRCS += $path"
#     }
# }
# catch {
#     Write-Error "ACPICA scan failed after $count files"
#     throw
# }

# $lines | Set-Content (Join-Path $root "acpica.mk") -Encoding utf8
# Write-Host "Generated acpica.mk"

# exit 0