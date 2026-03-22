# Script to copy built firmware binaries to firmware_release folder

param(
    [string]$environment = "esp32cam",
    [switch]$build = $false
)

$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildDir = Join-Path $projectRoot ".pio\build\$environment"
$releaseDir = Join-Path $projectRoot "firmware_release"

# Build first if requested
if ($build) {
    Write-Host "Building firmware for environment: $environment" -ForegroundColor Cyan
    platformio run --environment $environment
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        exit 1
    }
}

# Check if build directory exists
if (-not (Test-Path $buildDir)) {
    Write-Host "Build directory not found: $buildDir" -ForegroundColor Red
    exit 1
}

# Files to copy
$files = @(
    "bootloader.bin",
    "firmware.bin",
    "partitions.bin"
)

Write-Host "Copying firmware binaries to firmware_release..." -ForegroundColor Green

foreach ($file in $files) {
    $source = Join-Path $buildDir $file
    $destination = Join-Path $releaseDir $file
    
    if (Test-Path $source) {
        Copy-Item -Path $source -Destination $destination -Force
        Write-Host "✓ Copied $file" -ForegroundColor Green
    } else {
        Write-Host "✗ Source file not found: $file" -ForegroundColor Yellow
    }
}

Write-Host "Done!" -ForegroundColor Green
