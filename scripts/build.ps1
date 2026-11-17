[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [ValidateSet('all', 'android', 'firmware', 'vehicle-app', 'signs-app', 'uno', 'vehicle-esp32', 'sign-1', 'sign-2')]
    [string]$Target = 'all',

    [switch]$Offline
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$android = @{
    'vehicle-app' = 'mobile/vehicle-app'
    'signs-app' = 'mobile/traffic-signs-app'
}
$firmware = @{
    'uno' = @('arduino:avr:uno', 'firmware/vehicle/Arduino/ArduinoController')
    'vehicle-esp32' = @('esp32:esp32:esp32:FlashMode=dio,FlashFreq=80,PartitionScheme=no_ota', 'firmware/vehicle/ESP32/ElysiumRC')
    'sign-1' = @('esp32:esp32:XIAO_ESP32C3:PartitionScheme=no_ota', 'firmware/traffic-signs/traffic_sign_1')
    'sign-2' = @('esp32:esp32:XIAO_ESP32C3:PartitionScheme=noota_ffat', 'firmware/traffic-signs/traffic_sign_2')
}
$targets = switch ($Target) {
    'all' { 'vehicle-app', 'signs-app', 'uno', 'vehicle-esp32', 'sign-1', 'sign-2' }
    'android' { 'vehicle-app', 'signs-app' }
    'firmware' { 'uno', 'vehicle-esp32', 'sign-1', 'sign-2' }
    default { $Target }
}

$gradleArgs = @('--console=plain', 'test', 'assembleDebug')
if ($Offline) { $gradleArgs += '--offline' }

if ($targets | Where-Object { $android.ContainsKey($_) }) {
    $jdk = Join-Path $env:ProgramFiles 'Android/Android Studio/jbr'
    if (!$env:JAVA_HOME -and !(Get-Command java -ErrorAction SilentlyContinue) -and (Test-Path (Join-Path $jdk 'bin/java.exe'))) {
        $env:JAVA_HOME = $jdk
    }
    $sdk = Join-Path $env:LOCALAPPDATA 'Android/Sdk'
    if (!$env:ANDROID_HOME -and !$env:ANDROID_SDK_ROOT -and (Test-Path $sdk -PathType Container)) {
        $env:ANDROID_HOME = $sdk
    }
}

$failed = @()
foreach ($item in $targets) {
    Write-Host "=== $item ==="
    try {
        if ($android.ContainsKey($item)) {
            Push-Location (Join-Path $root $android[$item])
            try { & .\gradlew.bat @gradleArgs }
            finally { Pop-Location }
        } else {
            $cli = Get-Command arduino-cli -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
            $exe = if ($cli) { $cli.Source } else { Join-Path $env:LOCALAPPDATA 'Programs/Arduino CLI/arduino-cli.exe' }
            if (!(Test-Path -LiteralPath $exe -PathType Leaf)) { throw 'Nu gasesc arduino-cli. Vezi docs/HARDWARE_BASELINE.md.' }
            $board, $sketch = $firmware[$item]
            & $exe compile --fqbn $board (Join-Path $root $sketch)
        }
        if ($LASTEXITCODE -ne 0) { throw "Build exited with code $LASTEXITCODE." }
        Write-Host "PASS $item"
    } catch {
        $failed += $item
        Write-Warning "FAIL ${item}: $_"
    }
}
if ($failed.Count) {
    Write-Host "Failed targets: $($failed -join ', ')"
    exit 1
}
