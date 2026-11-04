$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$compiler = Get-ChildItem 'D:/STM32CubeIDE' -Filter arm-none-eabi-gcc.exe -Recurse | Select-Object -First 1 -ExpandProperty FullName
if (!$compiler) { throw 'STM32 GCC not found' }
$toolBin = Split-Path $compiler
$buildDir = Join-Path $PSScriptRoot 'verify-build'
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
Push-Location $projectRoot
try {
    $flags = @('-mcpu=cortex-m4', '-mfpu=fpv4-sp-d16', '-mfloat-abi=hard', '-mthumb', '--specs=nano.specs', '-g3', '-O0', '-ffunction-sections', '-fdata-sections', '-Wall', '-DDEBUG', '-DUSE_HAL_DRIVER', '-DSTM32G474xx')
    $includes = @('Core/Inc', 'Utils/Inc', 'Drivers/STM32G4xx_HAL_Driver/Inc', 'Drivers/STM32G4xx_HAL_Driver/Inc/Legacy', 'Drivers/CMSIS/Device/ST/STM32G4xx/Include', 'Drivers/CMSIS/Include') | ForEach-Object { '-I' + $_ }
    $sources = @(Get-Content 'Debug/objects.list' | ForEach-Object {
        $relative = $_.Trim('"').Replace('./', '')
        if ($relative -like 'Core/Startup/*') { $relative -replace '\.o$', '.s' }
        else { $relative -replace '\.o$', '.c' }
    }) + @(Get-ChildItem Utils/Src -Filter '*.c' | ForEach-Object { 'Utils/Src/' + $_.Name })
    $objects = @()
    foreach ($source in $sources) {
        $object = Join-Path $buildDir ($source.Replace('/', '_') + '.o')
        $extra = @()
        if ($source -like '*.c') { $extra = @('-std=gnu11') }
        & $compiler @flags @includes @extra -c $source -o $object
        if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $source" }
        $objects += $object
    }
    $elf = Join-Path $buildDir 'FSBB_Self.elf'
    $map = Join-Path $buildDir 'FSBB_Self.map'
    & $compiler @objects -o $elf '-mcpu=cortex-m4' '-mfpu=fpv4-sp-d16' '-mfloat-abi=hard' '-mthumb' '-TSTM32G474RETX_FLASH.ld' '--specs=nosys.specs' '--specs=nano.specs' "-Wl,-Map=$map" '-Wl,--gc-sections' '-static' '-Wl,--start-group' '-lc' '-lm' '-Wl,--end-group'
    if ($LASTEXITCODE -ne 0) { throw 'Link failed' }
    & "$toolBin/arm-none-eabi-size.exe" $elf
    & "$toolBin/arm-none-eabi-nm.exe" $elf | Select-String 'rawPara|adcPara|adcCount|HAL_ADCEx_InjectedConvCpltCallback'
    & "$toolBin/arm-none-eabi-gdb.exe" -nx -batch $elf -ex 'info address rawPara' -ex 'info address adcPara' -ex 'ptype rawPara' -ex 'ptype adcPara' -ex 'info line HAL_ADCEx_InjectedConvCpltCallback'
    if ($LASTEXITCODE -ne 0) { throw 'GDB validation failed' }
    Write-Output 'PASS: full Debug compilation, link, and offline GDB symbol/type lookup.'
} finally {
    Pop-Location
}
