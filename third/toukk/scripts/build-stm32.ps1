$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$cmake = "E:\CLion\CLion 2026.2.2\bin\cmake\win\x64\bin\cmake.exe"
$armBin = "E:\CLion\CLion 2026.2.2\arm\bin"
$make = "E:\CLion\CLion 2026.2.2\bin\mingw\bin\mingw32-make.exe"
$buildDir = Join-Path $projectRoot "cmake-build-debug-mingw-1"

if (-not (Test-Path -LiteralPath $cmake)) {
    throw "CMake was not found at $cmake"
}
if (-not (Test-Path -LiteralPath (Join-Path $armBin "arm-none-eabi-gcc.exe"))) {
    throw "ARM GCC was not found under $armBin"
}
if (-not (Test-Path -LiteralPath $make)) {
    throw "MinGW Make was not found at $make"
}

$env:PATH = "$armBin;$(Split-Path -Parent $make);$env:PATH"

Push-Location $projectRoot
try {
    & $cmake `
        -S $projectRoot `
        -B $buildDir `
        -G "MinGW Makefiles" `
        -DCMAKE_BUILD_TYPE=Debug `
        -DCMAKE_C_COMPILER="$armBin\arm-none-eabi-gcc.exe" `
        -DCMAKE_ASM_COMPILER="$armBin\arm-none-eabi-gcc.exe" `
        -DCMAKE_MAKE_PROGRAM="$make"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    & $cmake --build $buildDir --parallel
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
finally {
    Pop-Location
}
