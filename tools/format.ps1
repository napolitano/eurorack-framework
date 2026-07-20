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

foreach ($file in $files) {
    clang-format -i -style=file $file.FullName
}

Write-Host "Formatted $($files.Count) C++ files."
