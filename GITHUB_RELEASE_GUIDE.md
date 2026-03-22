# GitHub Release - Setup Instructions

## 📦 Creating a Release on GitHub

This guide explains how to create an official release on GitHub with the pre-built firmware binaries.

---

## 🎯 Release Package Contents

Your release should include these files:

### Binary Files (Required)
- `bootloader.bin` - ESP32 bootloader
- `partitions.bin` - Partition table with OTA support
- `firmware.bin` - Main application firmware

### Documentation Files (Required)
- `FLASH_INSTRUCTIONS.md` - Step-by-step flashing guide
- `RELEASE_NOTES.md` - Release notes with features and setup instructions
- `OTA_INSTRUCTIONS.md` - Wireless update instructions

### Optional Files
- `README.md` - Quick reference guide
- Source code (as .zip archive)

---

## 📋 Step-by-Step: Create GitHub Release

### 1. Navigate to Releases Page

1. Go to your GitHub repository
2. Click the **"Releases"** link on the right sidebar (or navigation)
3. Click **"Create a new release"** button

### 2. Fill in Release Information

**Tag version:**
```
v1.0
```
(or your version number: v1.0.1, v2.0, etc.)

**Release title:**
```
ESP32-CAM 4WD Robot Firmware v1.0
```

**Describe this release** (main content - use the RELEASE_NOTES.md content):

```markdown
# ESP32-CAM 4WD Camera Robot Firmware v1.0

**Release Date:** November 24, 2025

## 🚀 Quick Start

### Flash Using Web Flasher (Easiest!)
1. Download the three `.bin` files from this release
2. Go to: https://esptool.spacehuhn.com/
3. Select your device
4. Add files with addresses:
   - `bootloader.bin` → 0x1000
   - `partitions.bin` → 0x8000
   - `firmware.bin` → 0x10000
5. Click **Program** ✅

More details in FLASH_INSTRUCTIONS.md

## ✨ Features
- 🌐 WiFi AP mode for easy setup
- 📷 MJPEG video streaming
- 🎮 Web-based joystick control
- ⚡ UDP low-latency control
- 🔄 Over-The-Air (OTA) updates
- 🛑 Motor safety watchdog
- 📡 mDNS hostname: esp32-cam-4wd.local

## 📥 Downloads

Find the binary files below to download.

## 📚 Documentation
- **FLASH_INSTRUCTIONS.md** - Complete flashing guide (4 methods)
- **OTA_INSTRUCTIONS.md** - Wireless update procedures
- **RELEASE_NOTES.md** - Full release notes and troubleshooting

## ⚠️ Important
- Change default passwords before production use
- Motor auto-stops after 1 second (safety feature)
- Requires 5V 2A minimum power
- WiFi: 2.4GHz only (no 5GHz support)

See documentation files for setup and troubleshooting!
```

### 3. Upload Release Assets

**Before posting the release, upload the binary files:**

1. Scroll down to **"Attach binaries by dropping them here or selecting them"** section
2. Click to open file browser or drag-and-drop these files:
   - `bootloader.bin`
   - `partitions.bin`
   - `firmware.bin`
   - `FLASH_INSTRUCTIONS.md`
   - `RELEASE_NOTES.md`
   - `OTA_INSTRUCTIONS.md`
   - `README.md` (optional)

3. Wait for uploads to complete (GitHub shows progress)

### 4. Publish Release

1. Choose **"Publish release"** button (or "Save as draft" to review first)
2. GitHub will:
   - Create a tag for your version
   - Make release visible on Releases page
   - Allow users to download files
   - Show release in repository activity

---

## 📝 Release Title Convention

Use a consistent naming pattern:

```
ESP32-CAM 4WD Robot - Firmware v1.0
ESP32-CAM 4WD Robot - Firmware v1.0.1 (Bugfix)
ESP32-CAM 4WD Robot - Firmware v2.0 (Major Release)
```

---

## 📌 Release Description Template

Use this template for the description:

```markdown
# Version X.X.X

**Release Date:** Month DD, YYYY

## 🎯 What's New

- ✅ Feature 1
- ✅ Feature 2
- 🐛 Fixed issue 1
- 🐛 Fixed issue 2

## 🚀 Quick Start

### Web Flasher (Easiest)
1. Download `.bin` files from below
2. Go to: https://esptool.spacehuhn.com/
3. Select device and upload with addresses:
   - bootloader.bin → 0x1000
   - partitions.bin → 0x8000
   - firmware.bin → 0x10000
4. Click Program

### First Boot
- Connect to: ESP32-CAM-Setup WiFi
- Open: http://192.168.4.1
- Configure your network
- Access at: http://esp32-cam-4wd.local

## 📚 Documentation
- **FLASH_INSTRUCTIONS.md** - All flashing methods
- **OTA_INSTRUCTIONS.md** - Wireless updates
- **README.md** - Features and usage

## ⚠️ Important Notes
- Default passwords: AP=12345678, OTA=robot123
- Change for production use!
- Motors auto-stop after 1 second (safety)
- 5V 2A minimum power required

## 🔧 Hardware Required
- AI-Thinker ESP32-CAM
- USB cable or 7.4V LiPo battery
- 2.4GHz WiFi network

## 📥 Files in This Release
- `bootloader.bin` - Bootloader (do not modify)
- `partitions.bin` - Partition table (do not modify)
- `firmware.bin` - Main firmware
- Documentation and guides

**See RELEASE_NOTES.md for complete details!**
```

---

## 🏷️ Git Tag Naming

GitHub automatically creates tags when you publish releases. Recommended convention:

```
v1.0          - First major release
v1.0.1        - Patch/bugfix release
v1.1          - Minor feature release
v2.0          - Major revamp/breaking changes
```

Commands for reference:
```bash
# Create a git tag locally
git tag v1.0

# Push tag to GitHub
git push origin v1.0
```

---

## 🔗 Release URL Structure

After publishing, your release will be available at:

```
https://github.com/YOUR_USERNAME/YOUR_REPO/releases/tag/v1.0
```

Users can:
- See all release details
- Download binary files
- Access release notes
- View commit history for that version

---

## 📊 Best Practices

### Do's ✅
- ✅ Use semantic versioning (v1.0, v1.1, v2.0)
- ✅ Include all necessary binary files
- ✅ Provide clear setup instructions
- ✅ List new features and bug fixes
- ✅ Include safety warnings
- ✅ Link to documentation
- ✅ Keep release notes concise but complete

### Don'ts ❌
- ❌ Don't include source code in binaries (use separate archive)
- ❌ Don't forget to test binaries before releasing
- ❌ Don't release with default/insecure passwords unchanged
- ❌ Don't use confusing version numbering
- ❌ Don't forget documentation files
- ❌ Don't make releases without version tags

---

## 🔄 Future Releases

### Update Process

1. **Compile new firmware:**
   ```bash
   platformio run -e esp32cam
   ```

2. **New binaries automatically copied to `firmware_release/`** (via extra_script.py)

3. **Update CHANGELOG.md** with new changes

4. **Create new release on GitHub:**
   - Tag: v1.0.1 (or v2.0, etc.)
   - Upload new .bin files
   - Update RELEASE_NOTES.md
   - Publish

5. **Users can download immediately** - no compilation needed!

---

## 🎯 Release Checklist

Before publishing a release:

- [ ] All three binary files present (bootloader.bin, partitions.bin, firmware.bin)
- [ ] Tested on hardware (flashing works, firmware boots)
- [ ] FLASH_INSTRUCTIONS.md included and up-to-date
- [ ] RELEASE_NOTES.md created/updated
- [ ] OTA_INSTRUCTIONS.md included
- [ ] Default passwords documented
- [ ] Safety warnings included
- [ ] Troubleshooting section included
- [ ] Download links work (test after publishing)
- [ ] Release notes clearly describe what's new
- [ ] Version number follows semantic versioning
- [ ] Git tag created for version

---

## 📞 Example GitHub Release URL

After publishing, visit:
```
https://github.com/YOUR_USERNAME/ESP32_Cam_4WD_Robot_Car/releases
```

Users will see:
- Latest release highlighted
- All version history
- Download buttons for each file
- Release notes for each version

---

## 🚀 Ready to Deploy!

Once your release is published on GitHub:

1. **Share the link:** Users can find your release on the Releases page
2. **Direct download:** Users can download `.bin` files directly
3. **Easy setup:** Users follow FLASH_INSTRUCTIONS.md and web flasher
4. **Support:** Documentation provides all needed info

**Your firmware is now available for everyone! 🎉**

---

For questions about GitHub Releases, see: https://docs.github.com/en/repositories/releasing-projects-on-github
