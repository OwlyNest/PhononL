$failed = 0
$testnr = 0;
Write-Host "[X] test $($testnr++): #endif"
find Shadow -path '*.ps1' -prune -o -type f -print0 | xargs -0 grep -H "#endif" | grep -v '#endif /\* .* \*/' > /dev/null
if ($LASTEXITCODE -eq 0 || $LASTEXITCODE -ge 2) {
    $failed++;
}

if ($failed -gt 0) {
    Write-Error "[!] $failed tests failed"
    exit $failed
} else {
    Write-Host "[X] all tests passed"
    exit 0
}