$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$openocd = "E:\OpenOCD-20260302-0.12.0\bin\openocd.exe"
$firmware = Join-Path $projectRoot "cmake-build-debug-mingw-1\toukk.elf"
$openocdConfig = "E:\CLion\CLion 2026.2.2\CFJ_STM32\stm32f1.cfg"

if (-not (Test-Path -LiteralPath $openocd)) {
    throw "OpenOCD was not found at $openocd"
}
if (-not (Test-Path -LiteralPath $firmware)) {
    throw "Firmware was not found at $firmware. Run scripts\build-stm32.ps1 first."
}

$programCommand = "program {$firmware} verify reset exit"
& $openocd -f $openocdConfig -c $programCommand
exit $LASTEXITCODE
