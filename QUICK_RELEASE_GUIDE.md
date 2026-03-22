# Quick Release Publishing Guide

**Simple steps to create a GitHub release for your firmware. Takes ~5 minutes!**

---

## ⚡ The 5-Minute Release Process

### 1. Build the Firmware (1 minute)
```bash
platformio run -e esp32cam
```
✅ Binaries automatically copied to `firmware_release/` by extra_script.py

### 2. Update Docs (2 minutes)
- [ ] Update CHANGELOG.md
- [ ] Create/update RELEASE_NOTES.md in `firmware_release/`
- [ ] Keep FLASH_INSTRUCTIONS.md current

### 3. Create GitHub Release (2 minutes)

**Go to:** https://github.com/YOUR_USERNAME/YOUR_REPO/releases

**Click:** "Create a new release"

**Fill in:**
- **Tag:** `v1.0` (or v1.0.1, v2.0, etc.)
- **Title:** `ESP32-CAM 4WD Robot Firmware v1.0`
- **Description:** (see template below)

### 4. Upload Files

**Upload these from `firmware_release/` folder:**
```
✓ bootloader.bin
✓ partitions.bin
✓ firmware.bin
✓ FLASH_INSTRUCTIONS.md
✓ RELEASE_NOTES.md
✓ OTA_INSTRUCTIONS.md
```

### 5. Publish
**Click:** "Publish release" ✅

---

## 📝 Release Description Template

Paste this into the description field (replace placeholders):

```markdown
# Quick Start

## Download & Flash
1. Download the three `.bin` files below
2. Go to: https://esptool.spacehuhn.com/
3. Add files with addresses:
   - bootloader.bin → 0x1000
   - partitions.bin → 0x8000
   - firmware.bin → 0x10000
4. Click **Program** ✅

## First Boot
- Connect to: **ESP32-CAM-Setup** WiFi (password: 12345678)
- Open: http://192.168.4.1
- Configure your WiFi
- Access at: http://esp32-cam-4wd.local

## Features
- 📷 MJPEG camera streaming
- 🎮 Web-based joystick control
- ⚡ UDP low-latency commands
- 🔄 OTA wireless updates
- 🛑 Motor safety watchdog
- 📡 mDNS hostname support

## Documentation
- **FLASH_INSTRUCTIONS.md** - All flashing methods
- **OTA_INSTRUCTIONS.md** - Wireless update guide
- **RELEASE_NOTES.md** - Full details

## ⚠️ Before Use
- Change default passwords (12345678 & robot123)
- Requires 5V 2A power minimum
- 2.4GHz WiFi only
- See RELEASE_NOTES.md for troubleshooting

**See included documentation files for complete information!**
```

---

## 📋 File Addresses (Always Use These)

| File | Address | Example |
|------|---------|---------|
| bootloader.bin | 0x1000 | Must be this address |
| partitions.bin | 0x8000 | Must be this address |
| firmware.bin | 0x10000 | Must be this address |

❌ DO NOT change these addresses!

---

## 🆚 Version Numbering

Choose the right version for your release:

| Version | Use When | Example |
|---------|----------|---------|
| v1.0 | First release | Initial public version |
| v1.0.1 | Bugfix only | Minor fixes, no features |
| v1.1 | New feature | Feature additions |
| v2.0 | Major rewrite | Significant changes |

---

## ✅ Pre-Release Checklist

Before you publish:

- [ ] Tested flashing on real device
- [ ] Device boots successfully
- [ ] Camera works
- [ ] Motors work
- [ ] WiFi works
- [ ] All three .bin files present
- [ ] Documentation updated
- [ ] Default passwords documented
- [ ] Safety warnings included

---

## 🔗 GitHub Release URL

After publishing, your release is at:

```
https://github.com/YOUR_USERNAME/YOUR_REPO/releases/tag/v1.0
```

Users can:
- See your release notes
- Download .bin files
- View entire release history

---

## 🎯 Files Users Will Download

When users visit your release page, they'll find:

```
⬇️ bootloader.bin (17 KB)      - Click to download
⬇️ partitions.bin (3 KB)       - Click to download
⬇️ firmware.bin (988 KB)       - Click to download
⬇️ FLASH_INSTRUCTIONS.md       - Click to download
⬇️ RELEASE_NOTES.md            - Click to download
⬇️ OTA_INSTRUCTIONS.md         - Click to download
(Optional) Source code         - Auto-generated .zip
```

---

## ⚡ Quick Troubleshooting

### Files not uploading?
- Wait a few seconds between uploads
- Try uploading files one at a time
- Refresh page if it stalls

### Can't find Release page?
- Go to: https://github.com/YOUR_USERNAME/REPO/releases
- Or click "Releases" link on repo homepage

### Want to update a release?
- ❌ Don't modify existing release
- ✅ Create new version (v1.0.1 or v1.1)

### Accidentally published?
- Can delete release on GitHub
- Or publish as draft first to review

---

## 🚀 You're Done!

**What just happened:**
1. ✅ Your firmware is on GitHub
2. ✅ Users can download it without building
3. ✅ Clear documentation is included
4. ✅ Anyone can flash to their device
5. ✅ Support files are accessible

**Your firmware is now available to the world!** 🎉

---

## 📞 Next Steps

1. **Share the link:** Tell users about your release
2. **Monitor feedback:** Check GitHub issues/discussions
3. **Plan next release:** When to release v1.0.1 or v1.1?
4. **Maintain repository:** Keep documentation updated

---

## 📚 Detailed Guides

For more information, see:
- **GITHUB_RELEASE_GUIDE.md** - Complete GitHub release walkthrough
- **RELEASE_CHECKLIST.md** - Detailed pre-release checklist
- **RELEASE_PACKAGE_INFO.md** - File structure and organization
- **RELEASE_NOTES.md** - Template for release notes

---

## 🔄 Repeat for Next Release

Next time you want to release:

```bash
# 1. Make changes to code
# 2. Build
platformio run -e esp32cam

# 3. Update docs
# (Edit CHANGELOG.md, RELEASE_NOTES.md)

# 4. Create new GitHub release
# (Go to releases page, repeat steps above with v1.0.1 or v2.0)

# Done! 🎉
```

---

**Happy releasing!** 🚀
