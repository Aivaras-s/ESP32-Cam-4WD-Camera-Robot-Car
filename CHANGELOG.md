# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

### Documentation
- **Emphasized mDNS hostname**: Updated all documentation to recommend `esp32-cam-4wd.local` as primary access method for beginners (eliminates need to find IP address)
- **Updated README.md**: Added clear examples using mDNS hostname with IP address as fallback
- **Updated JOYSTICK_GUIDE.md**: Highlighted easy mDNS access method
- **Updated UDP_VIDEO_README.md**: Showed both mDNS and IP access methods
- **Updated VERSIONS_COMPARISON.md**: Added mDNS examples alongside IP-based access
- **Updated SECURITY_CHANGES.md**: Added note about mDNS hostname convenience

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
