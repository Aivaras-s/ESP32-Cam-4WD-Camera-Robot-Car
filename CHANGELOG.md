# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

(No unreleased changes at this time)

## [1.0] - November 24, 2025

Initial public release with pre-built firmware binaries and comprehensive documentation for end users.

### Documentation
- **Emphasized mDNS hostname**: Updated all documentation to recommend `esp32-cam-4wd.local` as primary access method for beginners (eliminates need to find IP address)
- **Updated README.md**: Added clear examples using mDNS hostname with IP address as fallback
- **Updated JOYSTICK_GUIDE.md**: Highlighted easy mDNS access method
- **Updated UDP_VIDEO_README.md**: Showed both mDNS and IP access methods
- **Updated VERSIONS_COMPARISON.md**: Added mDNS examples alongside IP-based access
- **Updated SECURITY_CHANGES.md**: Added note about mDNS hostname convenience
- **Web flasher instructions**: Added detailed guide for https://esptool.spacehuhn.com/ in FLASH_INSTRUCTIONS.md (Option 4) for easiest flashing without installation
- **GitHub Release guide**: Created GITHUB_RELEASE_GUIDE.md with step-by-step instructions for publishing releases
- **Release package documentation**: Created comprehensive release guides:
  - **RELEASE_NOTES.md**: Complete user-friendly release notes with features, setup, and troubleshooting
  - **QUICK_RELEASE_GUIDE.md**: 5-minute quick reference for creating releases
  - **RELEASE_CHECKLIST.md**: Detailed pre-release verification checklist
  - **RELEASE_PACKAGE_INFO.md**: File structure and binary package organization
- **Updated README.md**: Added "Quick Flash (No Installation Required)" section at beginning of Getting Started
- **Updated firmware_release/README.md**: Enhanced with GitHub Release section and web flasher as primary method

### Fixed
- **mDNS Service Error**: Fixed "Failed adding service http.tcp" error by properly ending previous mDNS instance before re-initialization on WiFi reconnect
- **DHCP Hostname**: Fixed DHCP hostname to match mDNS hostname (`esp32-cam-4wd` instead of `esp32-8E04FC`) by setting hostname before WiFi mode configuration
- **Firmware Release Binaries**: Added automatic post-build script to keep `firmware_release/` folder synchronized with latest compiled binaries

### Added
- **extra_script.py**: PlatformIO post-build script that automatically copies compiled firmware binaries to `firmware_release/` folder
- **update_firmware_release.ps1**: PowerShell utility script for manual firmware binary updates
- **extra_scripts configuration**: Added to both `esp32cam` and `esp32cam_ota` environments in platformio.ini
- **Release binary package**: Pre-built firmware files ready for users without compilation:
  - bootloader.bin (17 KB)
  - partitions.bin (3 KB)
  - firmware.bin (~988 KB)
- **Release resources**: Complete release documentation package for GitHub distribution

### Changed
- WiFi initialization order: Now sets hostname before entering WiFi modes (both STA and AP) for proper DHCP registration
- mDNS cleanup: Added explicit `MDNS.end()` call before re-initialization to prevent service duplication errors

## [Previous Releases]
See git history for earlier changes.
