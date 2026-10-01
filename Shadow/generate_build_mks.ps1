$ErrorActionPreference = "Stop"

# --------------------------------------------------------------------
# generate_build_mks.ps1
#
# Single source of truth: build_config.json
#
# For every subsystem declared there this script emits:
#
#   <dir>/build.mk
#
#     leaf       -> scanned source lists
#     aggregator -> scanned own sources + "-include" lines for
#                   declared/discovered child directories
#
# The hierarchy can be arbitrarily deep.
#
# Example:
#
#   BUS/
#     PCI/
#       ACPI/
#       Bridge/
#       Core/
#       Host/
#
# can be represented as:
#
#   {
#       "dir": "BUS",
#       "subdirs": [
#           {
#               "dir": "PCI",
#               "subdirs": [
#                   { "dir": "ACPI" },
#                   { "dir": "Bridge" },
#                   { "dir": "Core" },
#                   { "dir": "Host" }
#               ]
#           }
#       ]
#   }
#
# Or:
#
#   {
#       "dir": "BUS",
#       "subdirs": [
#           {
#               "dir": "PCI",
#               "subdirs": [ "auto" ]
#           }
#       ]
#   }
#
# "auto" discovers every immediate child directory at that level.
#
# Top level:
#
#   build/subsystems.mk
#
# contains:
#   -include <top-level-dir>/build.mk
#   BUILD_DEFINES
#
# -D macros are NOT declared in the config — they are derived from
# the directory path, so they can never drift out of sync or collide:
#
#   Boot         -> -D__BOOT__
#   DRV          -> -D__DRV__
#   DRV/SERIAL   -> -D__DRV_SERIAL__
#   BUS/PCI/ACPI -> -D__BUS_PCI_ACPI__
#
# Only genuinely global, non-directory defines (like __OWLYNEST__)
# live in the config's "global_defines".
#
# The makefile -includes build/subsystems.mk and has a rule to
# regenerate it, so it never goes stale.
# --------------------------------------------------------------------

$root = Join-Path $PSScriptRoot ""
$configPath = Join-Path $root "build_config.json"

if (-not (Test-Path $configPath)) {
    throw "Build config not found: $configPath"
}

$config = Get-Content $configPath -Raw | ConvertFrom-Json


# --------------------------------------------------------------------
# Helpers
# --------------------------------------------------------------------

function Get-RelativePath {
    param([string]$Path)

    return [System.IO.Path]::GetRelativePath(
        $root,
        [System.IO.Path]::GetFullPath($Path)
    ).Replace("\", "/")
}


function Get-DirDefine {
    # BUS/PCI/ACPI -> __BUS_PCI_ACPI__
    #
    # The define is always derived from the complete directory path.
    # It is therefore impossible for the config and define to disagree.

    param([string]$RelDir)

    $flat = ($RelDir -replace "[/\\]", "_").ToUpper()

    return "-D__${flat}__"
}


function Get-SourceLines {
    # Scans ONE directory only.
    #
    # Recursion is handled by the build.mk hierarchy, not here.

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
        Get-ChildItem $FullDir `
            -Filter $entry.Filter `
            -File `
            -ErrorAction SilentlyContinue |
            Sort-Object Name |
            ForEach-Object {
                $lines += "$($entry.Var) += $(Get-RelativePath $_.FullName)"
            }
    }

    return $lines
}


function Get-ChildDirs {
    # Discovers immediate child directories of RelDir.
    #
    # Only used for:
    #
    #   "subdirs": [ "auto" ]
    #
    # This deliberately does NOT recurse.

    param([string]$RelDir)

    $fullDir = Join-Path $root ($RelDir -replace "/", "\")

    if (-not (Test-Path $fullDir)) {
        return @()
    }

    return @(
        Get-ChildItem $fullDir `
            -Directory `
            -ErrorAction SilentlyContinue |
            Sort-Object Name |
            Select-Object -ExpandProperty Name
    )
}


# --------------------------------------------------------------------
# Recursive build.mk generation
# --------------------------------------------------------------------

function Write-BuildMk {
    <#
        Recursively generates build.mk files.

        Every node has:
            dir
            optional subdirs

        subdirs can contain either:

            "NAME"

        for a simple child directory, or:

            {
                "dir": "NAME",
                "subdirs": [...]
            }

        for a recursively configured child.

        A node without subdirs is a leaf.

        A node with subdirs is an aggregator, but it can ALSO have
        source files of its own.
    #>

    param(
        [Parameter(Mandatory)]
        [string]$RelDir,

        [Parameter(Mandatory)]
        [object]$Node
    )

    $fullDir = Join-Path $root ($RelDir -replace "/", "\")

    if (-not (Test-Path $fullDir)) {
        Write-Warning "Declared directory '$RelDir' does not exist — skipping."
        return
    }

    $localDefines = @()
    $localDefines += Get-DirDefine $RelDir

    # ---------------------------------------------------------------
    # Determine whether this node has children.
    # ---------------------------------------------------------------

    $hasSubdirs =
        $null -ne $Node.PSObject.Properties["subdirs"] -and
        $null -ne $Node.subdirs


    $children = @()

    if ($hasSubdirs) {

        $configuredSubdirs = @($Node.subdirs)

        # -----------------------------------------------------------
        # "auto" means discover every immediate child directory.
        #
        # We intentionally only support "auto" as the sole entry.
        # This avoids ambiguous combinations such as:
        #
        #   "subdirs": [ "auto", "Special" ]
        #
        # where Special would already be discovered by auto.
        # -----------------------------------------------------------

        if (
            $configuredSubdirs.Count -eq 1 -and
            [string]$configuredSubdirs[0] -eq "auto"
        ) {
            $children = Get-ChildDirs $RelDir
        }
        else {
            foreach ($child in $configuredSubdirs) {

                # ---------------------------------------------------
                # Backwards-compatible short form:
                #
                #   "subdirs": [ "SERIAL", "PS2" ]
                #
                # becomes a child node with no children of its own.
                # ---------------------------------------------------

                if ($child -is [string]) {
                    $children += [PSCustomObject]@{
                        dir = $child
                    }
                }
                else {
                    # ------------------------------------------------
                    # Recursive object form:
                    #
                    # {
                    #     "dir": "PCI",
                    #     "subdirs": [...]
                    # }
                    # ------------------------------------------------

                    $children += $child
                }
            }
        }
    }
    


    # ---------------------------------------------------------------
    # Generate this directory's build.mk.
    # ---------------------------------------------------------------

    $lines = @()

    if ($children.Count -gt 0) {
        $lines += "# Auto-generated aggregator for $RelDir — do not edit."
        $lines += "# Add/remove subsystems in build_config.json, not here."
    }
    else {
        $lines += "# Auto-generated build.mk for $RelDir — do not edit."
        $lines += "# Regenerate via generate_build_mks.ps1 (config: build_config.json)."
    }

    $lines += ""


    # ---------------------------------------------------------------
    # Own sources.
    #
    # These are always included, even when this directory is an
    # aggregator.
    # ---------------------------------------------------------------

    $own = Get-SourceLines $fullDir

    if ($own.Count -gt 0) {
        $lines += $own

        if ($children.Count -gt 0) {
            $lines += ""
        }
    }


    # ---------------------------------------------------------------
    # Recursively generate children and include them.
    # ---------------------------------------------------------------

foreach ($child in $children) {
        $childName = [string]$child.dir
        if ([string]::IsNullOrWhiteSpace($childName)) {
            throw "Invalid subsystem entry under '$RelDir': missing 'dir'."
        }

        $childRelDir = "$RelDir/$childName"

        # Capture the defines returned by the recursive call
        $childDefines = Write-BuildMk -RelDir $childRelDir -Node $child
        $localDefines += $childDefines

        $lines += "-include $childRelDir/build.mk"
    }


    # ---------------------------------------------------------------
    # Write this build.mk.
    # ---------------------------------------------------------------

    $buildMkPath = Join-Path $fullDir "build.mk"

    $lines | Set-Content $buildMkPath -Encoding utf8

    if ($children.Count -gt 0) {
        Write-Host "Generated $RelDir/build.mk (aggregator, $(Get-DirDefine $RelDir))"
    }
    else {
        Write-Host "Generated $RelDir/build.mk ($(Get-DirDefine $RelDir))"
    }

    return $localDefines
}


# --------------------------------------------------------------------
# Walk the top-level config
# --------------------------------------------------------------------

$includes = @()
$defines  = @()

# Global defines are explicitly declared.
foreach ($d in @($config.global_defines)) {
    $defines += "-D$d"
}


foreach ($sub in @($config.subsystems)) {

    $dir = [string]$sub.dir

    if ([string]::IsNullOrWhiteSpace($dir)) {
        throw "Invalid top-level subsystem: missing 'dir'."
    }

    # Every directory gets a define.
    #
    # For example:
    #
    #   BUS       -> __BUS__
    #   BUS/PCI   -> __BUS_PCI__
    #   BUS/PCI/X -> __BUS_PCI_X__
    #
    $subtreeDefines = Write-BuildMk -RelDir $dir -Node $sub
    $defines += $subtreeDefines


    # Top-level include.
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

New-Item `
    -ItemType Directory `
    -Force `
    -Path $buildDir |
    Out-Null


$mk | Set-Content `
    (Join-Path $buildDir "subsystems.mk") `
    -Encoding utf8


Write-Host "Generated build/subsystems.mk"
Write-Host "Defines: $($defines -join " ")"


# --------------------------------------------------------------------
# ACPICA
# --------------------------------------------------------------------

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