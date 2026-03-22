# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Fixed
- **mDNS Service Error**: Fixed "Failed adding service http.tcp" error by properly ending previous mDNS instance before re-initialization on WiFi reconnect
- **DHCP Hostname**: Fixed DHCP hostname to match mDNS hostname (`esp32-cam-4wd` instead of `esp32-8E04FC`) by setting hostname before WiFi mode configuration
- **Firmware Release Binaries**: Added automatic post-build script to keep `firmware_release/` folder synchronized with latest compiled binaries

### Added
- **extra_script.py**: PlatformIO post-build script that automatically copies compiled firmware binaries to `firmware_release/` folder
- **update_firmware_release.ps1**: PowerShell utility script for manual firmware binary updates
- **extra_scripts configuration**: Added to both `esp32cam` and `esp32cam_ota` environments in platformio.ini

### Changed
- WiFi initialization order: Now sets hostname before entering WiFi modes (both STA and AP) for proper DHCP registration
- mDNS cleanup: Added explicit `MDNS.end()` call before re-initialization to prevent service duplication errors

## [Previous Releases]
See git history for earlier changes.
