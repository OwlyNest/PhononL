$ErrorActionPreference = "Stop"

# --------------------------------------------------------------------
# generate_build_mks.ps1
#
# Single source of truth: build_config.json
#
# For every subsystem declared there this script emits:
#   <dir>/build.mk        leaf       -> scanned source lists
#                         aggregator -> "-include" lines for declared
#                                       subdirs (+ own-dir sources)
#   build/subsystems.mk   top level  -> "-include <dir>/build.mk" for
#                                       each declared subsystem, plus
#                                       BUILD_DEFINES from "macros".
#
# The makefile -includes build/subsystems.mk and has a rule to
# regenerate it, so it never goes stale.
#
# Subsystems may declare subdirectories:
#   "subdirs": [ "SERIAL" ]                          plain
#   "subdirs": [ { "name": "SERIAL",
#                  "macros": [ "__SERIAL__" ] } ]    with defines
#   "subdirs": [ "auto" ]                            discover all
#                                                      child dirs
# --------------------------------------------------------------------

$root = Join-Path $PSScriptRoot ""
$configPath = Join-Path $root "build_config.json"

if (-not (Test-Path $configPath)) {
    throw "Build config not found: $configPath"
}

$config = Get-Content $configPath -Raw | ConvertFrom-Json

function Get-RelativePath {
    param([string]$Path)

    return [System.IO.Path]::GetRelativePath(
        $root,
        [System.IO.Path]::GetFullPath($Path)
    ).Replace("\", "/")
}

function Get-SourceLines {
    # Scans one directory (non-recursive) and returns the
    # "C_SRCS += ..." style lines for every known source extension.
    param([string]$FullDir)

    $lines = @()

    $map = @(
        @{ Filter = "*.C";   Var = "C_SRCS"    },
        @{ Filter = "*.CPP"; Var = "CPP_SRCS"  },
        @{ Filter = "*.ASM"; Var = "ASM_SRCS"  },
        @{ Filter = "*.S";   Var = "GAS_SRCS"  },
        @{ Filter = "*.rs";  Var = "RUST_SRCS" },
        @{ Filter = "*.c3";  Var = "C3_SRCS"   }
    )

    foreach ($entry in $map) {
        Get-ChildItem $FullDir -Filter $entry.Filter -File -ErrorAction SilentlyContinue |
            Sort-Object Name |
            ForEach-Object {
                $lines += "$($entry.Var) += $(Get-RelativePath $_.FullName)"
            }
    }

    return $lines
}

function Write-LeafBuildMk {
    param([Parameter(Mandatory)][string]$RelDir)

    $fullDir = Join-Path $root ($RelDir -replace "/", "\")
    if (-not (Test-Path $fullDir)) {
        Write-Warning "Declared directory '$RelDir' does not exist. Skipping."
        return
    }

    $lines = @(
        "# Auto-generated build.mk for $RelDir. Do not edit.",
        "# Regenerate via generate_build_mks.ps1 (config: build_config.json).",
        ""
    ) + (Get-SourceLines $fullDir)

    $lines | Set-Content (Join-Path $fullDir "build.mk") -Encoding utf8
    Write-Host "Generated $RelDir/build.mk"
}

function Write-AggregatorBuildMk {
    # A directory that declares "subdirs" gets a build.mk that pulls in
    # exactly those subdirectories, plus its own direct sources, if any.
    # $SubDirs entries are objects: @{ Name = ...; Macros = @(...) }.
    param(
        [Parameter(Mandatory)][string]$RelDir,
        [Parameter(Mandatory)][object[]]$SubDirs
    )

    $fullDir = Join-Path $root ($RelDir -replace "/", "\")
    if (-not (Test-Path $fullDir)) {
        Write-Warning "Declared directory '$RelDir' does not exist. Skipping."
        return
    }

    $lines = @(
        "# Auto-generated aggregator for $RelDir. Do not edit.",
        "# Add/remove subsystems in build_config.json, not here.",
        ""
    )

    $own = Get-SourceLines $fullDir
    if ($own.Count -gt 0) {
        $lines += $own
        $lines += ""
    }

    foreach ($sub in $SubDirs) {
        Write-LeafBuildMk "$RelDir/$($sub.Name)"
        $lines += "-include $RelDir/$($sub.Name)/build.mk"
    }

    $lines | Set-Content (Join-Path $fullDir "build.mk") -Encoding utf8
    Write-Host "Generated $RelDir/build.mk (aggregator)"
}

# --------------------------------------------------------------------
# Walk the config
# --------------------------------------------------------------------

$includes = @()
$defines  = @()

foreach ($d in @($config.global_defines)) {
    $defines += "-D$d"
}

foreach ($sub in $config.subsystems) {
    $dir = $sub.dir

    foreach ($m in @($sub.macros)) {
        $defines += "-D$m"
    }

    $hasSubdirs = $null -ne $sub.PSObject.Properties["subdirs"] -and $sub.subdirs

    if ($hasSubdirs) {
        $raw = @($sub.subdirs)

        if ($raw.Count -eq 1 -and $raw[0] -eq "auto") {
            # Discover every child directory, nothing to declare by hand.
            $raw = @(Get-ChildItem (Join-Path $root $dir) -Directory `
                        -ErrorAction SilentlyContinue |
                     Sort-Object Name | Select-Object -ExpandProperty Name)
        }

        # Normalise: strings stay plain, objects may carry macros.
        $subs = @()
        foreach ($r in $raw) {
            if ($r -is [string]) {
                $subs += @{ Name = $r; Macros = @() }
            }
            else {
                $subs += @{ Name = $r.name; Macros = @($r.macros) }
                foreach ($m in @($r.macros)) {
                    $defines += "-D$m"
                }
            }
        }

        Write-AggregatorBuildMk -RelDir $dir -SubDirs $subs
    }
    else {
        Write-LeafBuildMk $dir
    }

    $includes += "-include $dir/build.mk"
}

# --------------------------------------------------------------------
# Top-level file the makefile actually includes
# --------------------------------------------------------------------

$mk = @(
    "# Auto-generated by generate_build_mks.ps1 from build_config.json.",
    "# Do not edit; regenerate with generate_build_mks.ps1.",
    ""
) + $includes

if ($defines.Count -gt 0) {
    $mk += ""
    $mk += "BUILD_DEFINES += $($defines -join " ")"
}

$buildDir = Join-Path $root "build"
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$mk | Set-Content (Join-Path $buildDir "subsystems.mk") -Encoding utf8

Write-Host "Generated build/subsystems.mk"
Write-Host "Defines: $($defines -join " ")"

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