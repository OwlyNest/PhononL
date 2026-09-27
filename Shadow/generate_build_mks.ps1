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
#                                       BUILD_DEFINES.
#
# -D macros are NOT declared in the config — they are derived from the
# directory path, so they can never drift out of sync or collide:
#   Boot         -> -D__BOOT__
#   DRV          -> -D__DRV__
#   DRV/SERIAL   -> -D__DRV_SERIAL__
# (dir1/common vs dir2/common becomes __DIR1_COMMON__ / __DIR2_COMMON__)
#
# Only genuinely global, non-directory defines (like __OWLYNEST__) live
# in the config's "global_defines".
#
# The makefile -includes build/subsystems.mk and has a rule to
# regenerate it, so it never goes stale.
#
# Subdirectory styles:
#   "subdirs": [ "SERIAL" ]   plain
#   "subdirs": [ "auto" ]     discover every child directory
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

function Get-DirDefine {
    # DRV/SERIAL -> __DRV_SERIAL__ — derived, never declared by hand.
    param([string]$RelDir)

    $flat = ($RelDir -replace "[/\\]", "_").ToUpper()
    return "-D__${flat}__"
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
        Write-Warning "Declared directory '$RelDir' does not exist — skipping."
        return
    }

    $lines = @(
        "# Auto-generated build.mk for $RelDir — do not edit.",
        "# Regenerate via generate_build_mks.ps1 (config: build_config.json).",
        ""
    ) + (Get-SourceLines $fullDir)

    $lines | Set-Content (Join-Path $fullDir "build.mk") -Encoding utf8
    Write-Host "Generated $RelDir/build.mk ($(Get-DirDefine $RelDir))"
}

function Write-AggregatorBuildMk {
    # A directory that declares "subdirs" gets a build.mk that pulls in
    # exactly those subdirectories — plus its own direct sources, if any.
    param(
        [Parameter(Mandatory)][string]$RelDir,
        [Parameter(Mandatory)][string[]]$SubDirs
    )

    $fullDir = Join-Path $root ($RelDir -replace "/", "\")
    if (-not (Test-Path $fullDir)) {
        Write-Warning "Declared directory '$RelDir' does not exist — skipping."
        return
    }

    $lines = @(
        "# Auto-generated aggregator for $RelDir — do not edit.",
        "# Add/remove subsystems in build_config.json, not here.",
        ""
    )

    $own = Get-SourceLines $fullDir
    if ($own.Count -gt 0) {
        $lines += $own
        $lines += ""
    }

    foreach ($sub in $SubDirs) {
        Write-LeafBuildMk "$RelDir/$sub"
        $lines += "-include $RelDir/$sub/build.mk"
    }

    $lines | Set-Content (Join-Path $fullDir "build.mk") -Encoding utf8
    Write-Host "Generated $RelDir/build.mk (aggregator, $(Get-DirDefine $RelDir))"
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
    $defines += Get-DirDefine $dir

    $hasSubdirs = $null -ne $sub.PSObject.Properties["subdirs"] -and $sub.subdirs

    if ($hasSubdirs) {
        $subs = @($sub.subdirs)

        if ($subs.Count -eq 1 -and $subs[0] -eq "auto") {
            # Discover every child directory — nothing to declare by hand.
            $subs = @(Get-ChildItem (Join-Path $root $dir) -Directory `
                        -ErrorAction SilentlyContinue |
                     Sort-Object Name | Select-Object -ExpandProperty Name)
        }

        foreach ($s in $subs) {
            $defines += Get-DirDefine "$dir/$s"
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
    "# Do not edit, regenerate with generate_build_mks.ps1.",
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