$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$files = Get-ChildItem `
    -Path "$projectRoot\libraries", "$projectRoot\tests", "$projectRoot\examples" `
    -Recurse `
    -File `
    -Include *.hpp, *.cpp

if (-not (Get-Command clang-format -ErrorAction SilentlyContinue)) {
    throw "clang-format is not available in PATH."
}

$failed = $false
foreach ($file in $files) {
    clang-format --dry-run --Werror -style=file $file.FullName
    if ($LASTEXITCODE -ne 0) {
        $failed = $true
    }
}

if ($failed) {
    throw "One or more files do not match .clang-format."
}

Write-Host "All $($files.Count) C++ files match .clang-format."
