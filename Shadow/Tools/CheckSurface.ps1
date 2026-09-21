<#*
    Shadow/Tools/CheckSurface.ps1

    Verifies that every non-static function definition belonging to an
    xSCAL subsystem is declared by that subsystem's XAL manifest, and
    that every XAL_METHOD declaration has a corresponding non-static
    function definition.

    Exit code:
      0 = all surfaces match
      1 = one or more violations
#>

$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$Violations = [System.Collections.Generic.List[object]]::new()

function Get-XalDeclaredFunctions {
    param(
        [Parameter(Mandatory)]
        [string]$Path
    )

    $Class = $null
    $Declared = [System.Collections.Generic.HashSet[string]]::new()

    foreach ($Line in Get-Content -LiteralPath $Path) {

        # XAL_META(*Class*, xSCAL)
        if ($Line -match 'XAL_META\s*\(\s*\*Class\*\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)') {
            $Class = $Matches[1]
        }

        # XAL_METHOD(..., FunctionName, ...)
        if ($Line -match '^\s*XAL_METHOD\s*\(\s*[^,]+,\s*([A-Za-z_][A-Za-z0-9_]*)\s*,') {
            [void]$Declared.Add($Matches[1])
        }
    }

    [PSCustomObject]@{
        Class    = $Class
        Declared = $Declared
    }
}

function Get-CFunctionDefinitions {
    param(
        [Parameter(Mandatory)]
        [string]$Path
    )

    $Text = Get-Content -LiteralPath $Path -Raw

    # Deliberately conservative C-function-definition matcher.
    #
    # Captures:
    #   optional calling-convention / qualifier words
    #   return type
    #   function name
    #   parameter list
    #   opening brace
    #
    # Does not attempt to be a complete C parser.

    $Pattern = @'
(?ms)
(?<![\w])
(?<prefix>
    (?:(?:static|extern|inline|__forceinline|__cdecl|__stdcall|__fastcall)\s+)*
)
(?<return>
    [A-Za-z_][A-Za-z0-9_]*
    (?:\s+|\s*\*\s*)+
)
(?<name>
    [A-Za-z_][A-Za-z0-9_]*
)
\s*
\(
    [^;{}]*?
\)
\s*
\{
'@

    foreach ($Match in [regex]::Matches(
        $Text,
        $Pattern,
        [System.Text.RegularExpressions.RegexOptions]::IgnorePatternWhitespace
    )) {
        $Prefix = $Match.Groups["prefix"].Value

        [PSCustomObject]@{
            Name   = $Match.Groups["name"].Value
            Static = $Prefix -match '\bstatic\b'
            File   = $Path
        }
    }
}

foreach ($XalFile in Get-ChildItem -Path $RepoRoot -Recurse -Filter "*.xal") {

    $SubsystemDir  = $XalFile.DirectoryName
    $SubsystemName = $XalFile.Directory.Name

    $Manifest = Get-XalDeclaredFunctions -Path $XalFile.FullName

    if ($Manifest.Class -ne "xSCAL") {
        continue
    }

    $Declared = $Manifest.Declared

    Write-Host "[x] Checking $SubsystemName ($($Declared.Count) declared)..."

    $Actual = [System.Collections.Generic.HashSet[string]]::new()

    foreach ($CFile in Get-ChildItem `
        -Path $SubsystemDir `
        -Recurse `
        -Filter "*.C" `
        -File) {

        foreach ($Function in Get-CFunctionDefinitions -Path $CFile.FullName) {

            if (-not $Function.Static) {
                [void]$Actual.Add($Function.Name)
            }
        }
    }

    # Actual but not declared.
    foreach ($Name in $Actual) {

        if (-not $Declared.Contains($Name)) {

            $Violations.Add(
                [PSCustomObject]@{
                    Subsystem = $SubsystemName
                    Kind      = "Undeclared public function"
                    Name      = $Name
                    Detail    = "non-static definition exists but is missing from $($XalFile.Name)"
                }
            )
        }
    }

    # Declared but not actual.
    foreach ($Name in $Declared) {

        if (-not $Actual.Contains($Name)) {

            $Violations.Add(
                [PSCustomObject]@{
                    Subsystem = $SubsystemName
                    Kind      = "Stale manifest entry"
                    Name      = $Name
                    Detail    = "declared in $($XalFile.Name) but no matching non-static definition was found"
                }
            )
        }
    }
}

if ($Violations.Count -eq 0) {
    Write-Host "[x] Every xSCAL surface matches its manifest."
    exit 0
}

Write-Host ""
Write-Host "[!] $($Violations.Count) surface violation(s):" -ForegroundColor Red

foreach ($Violation in $Violations) {
    Write-Host "  [$($Violation.Subsystem)] $($Violation.Kind): $($Violation.Name)"
    Write-Host "      $($Violation.Detail)"
}

exit 1