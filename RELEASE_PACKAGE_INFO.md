# Release Package - File Structure

## 📦 What Gets Released on GitHub

This document explains what files are included in the GitHub release and where they come from.

---

## 🎯 GitHub Release Contents

### Required: Binary Files (from `firmware_release/`)

```
bootloader.bin          ← Flashed at address 0x1000
partitions.bin          ← Flashed at address 0x8000  
firmware.bin            ← Flashed at address 0x10000
```

**What they are:**
- `bootloader.bin` - ESP32 bootloader (handles startup and OTA)
- `partitions.bin` - Partition table defining memory layout
- `firmware.bin` - Main application firmware (~988 KB)

**How they're created:**
- Automatically compiled when you run: `platformio run -e esp32cam`
- Automatically copied to `firmware_release/` via `extra_script.py`
- Ready to use - no modifications needed

---

### Recommended: Documentation Files

Include these in the release for instructions:

| File | Location | Purpose |
|------|----------|---------|
| FLASH_INSTRUCTIONS.md | firmware_release/ | How to flash (4 methods) |
| RELEASE_NOTES.md | firmware_release/ | Features, setup, troubleshooting |
| OTA_INSTRUCTIONS.md | firmware_release/ | How to update wirelessly |
| README.md | firmware_release/ | Quick reference |

**Why include them:**
- Users need instructions to flash
- Solves troubleshooting questions
- Provides complete documentation in one place

---

### Optional: Additional Files

```
firmware_release/
├── source-code.zip          ← Entire source code
├── partitions_ota.csv       ← Partition definition (reference)
└── [other documentation]    ← Any supplementary files
```

---

## 📂 File Organization

### In Your Workspace

```
ESP32 Cam 4WD Camera Robot Car/
├── src/
│   └── robotuko_kodas.cpp          ← Source code
├── platformio.ini                   ← Build configuration
├── extra_script.py                  ← Auto-copy binaries
├── CHANGELOG.md                     ← Version history
├── README.md                        ← Main documentation
├── GITHUB_RELEASE_GUIDE.md          ← This guides
├── RELEASE_CHECKLIST.md             ← Prepare releases
└── firmware_release/
    ├── bootloader.bin               ← Binary files
    ├── partitions.bin               ← (auto-created)
    ├── firmware.bin                 ← 
    ├── FLASH_INSTRUCTIONS.md        ← Documentation
    ├── RELEASE_NOTES.md             ← (Create for each release)
    ├── OTA_INSTRUCTIONS.md          ← Update guide
    └── README.md                    ← Quick start
```

### On GitHub Release Page

```
Release v1.0
├── Source code (zip)
├── bootloader.bin
├── partitions.bin
├── firmware.bin
├── FLASH_INSTRUCTIONS.md
├── RELEASE_NOTES.md
├── OTA_INSTRUCTIONS.md
└── README.md
```

---

## 🔄 Release Workflow

### Step 1: Prepare Code
Edit `src/robotuko_kodas.cpp` with new features/fixes

### Step 2: Build
```bash
platformio run -e esp32cam
```

**This automatically:**
- Compiles firmware
- Copies binaries to `firmware_release/` via `extra_script.py`
- Creates bootloader.bin, partitions.bin, firmware.bin

### Step 3: Update Documentation
- [ ] Update CHANGELOG.md
- [ ] Update RELEASE_NOTES.md (or create new version)
- [ ] Review FLASH_INSTRUCTIONS.md
- [ ] Update OTA_INSTRUCTIONS.md if needed

### Step 4: Create GitHub Release
- Go to: https://github.com/YOUR_USERNAME/YOUR_REPO/releases
- Click "Create new release"
- Tag: v1.0
- Title: "ESP32-CAM 4WD Robot Firmware v1.0"
- Description: (Use RELEASE_NOTES.md content)
- Upload files from `firmware_release/` folder

### Step 5: Publish
Click "Publish release" - Done!

---

## 📋 Files to Upload to GitHub

When creating a GitHub release, upload these files as assets:

### From `firmware_release/` folder:

**Required (Binary Files):**
1. `bootloader.bin`
2. `partitions.bin`
3. `firmware.bin`

**Documentation (Recommended):**
4. `FLASH_INSTRUCTIONS.md`
5. `RELEASE_NOTES.md`
6. `OTA_INSTRUCTIONS.md`
7. `README.md`

**Optional:**
8. `source-code.zip` (entire project)
9. `partitions_ota.csv` (reference)

---

## ✅ Verification Checklist

Before uploading, verify:

- [ ] All three .bin files present in `firmware_release/`
- [ ] File sizes reasonable:
  - bootloader.bin: ~17 KB
  - partitions.bin: ~3 KB
  - firmware.bin: ~988 KB (may vary)
- [ ] Files are recent (modtime = today's date)
- [ ] Documentation files are updated
- [ ] No temporary files included
- [ ] No personal/sensitive data in files

---

## 🔗 Download Links

After publishing release on GitHub:

**Direct downloads available at:**
```
https://github.com/YOUR_USERNAME/YOUR_REPO/releases/v1.0
```

**Users can download:**
- Individual files
- Entire release as .zip
- View release notes

---

## 🚀 For Users

Users downloading your release will:

1. Download the three .bin files
2. Read FLASH_INSTRUCTIONS.md
3. Use web flasher: https://esptool.spacehuhn.com/
4. Upload files with correct addresses
5. Flash to their device
6. Follow setup in RELEASE_NOTES.md

**All without compiling!** ✨

---

## 💾 Storage & Backup

### GitHub Stores:
- All release files permanently
- All versions accessible
- All release notes
- Download statistics
- Release tags in git

### You Should Keep:
- Source code in git repository
- CHANGELOG.md updated
- Binaries in `firmware_release/` for latest version
- Old releases (GitHub keeps them)

---

## 🔀 File Naming Convention

Keep naming consistent across releases:

```
bootloader.bin      ← Always this name
partitions.bin      ← Always this name
firmware.bin        ← Always this name
```

Do NOT include version in .bin filename:
- ❌ firmware-v1.0.bin
- ✅ firmware.bin

(Version in release tag/folder structure only)

---

## 📝 Release Descriptions

### In firmware_release/README.md

Include summary of what's in the release:
```markdown
## Release Contents

- bootloader.bin (17 KB) - ESP32 bootloader
- partitions.bin (3 KB) - Partition table
- firmware.bin (988 KB) - Main application

See FLASH_INSTRUCTIONS.md for how to flash.
```

### In GitHub Release Page

Use RELEASE_NOTES.md content or create summary:
```markdown
# Quick Start
1. Download .bin files
2. Go to https://esptool.spacehuhn.com/
3. Upload with addresses shown in FLASH_INSTRUCTIONS.md
4. Click Program
```

---

## 🎯 Common Usage Scenarios

### User wants to flash device:
- ✅ Download .bin files from GitHub release
- ✅ Read FLASH_INSTRUCTIONS.md
- ✅ Uses web flasher
- ❌ No compilation needed

### Developer wants source code:
- ✅ Download source-code.zip from release (optional)
- ✅ Or clone repository
- ✅ Compile with PlatformIO

### User wants OTA update:
- ✅ Read OTA_INSTRUCTIONS.md
- ✅ Download new firmware from new release
- ✅ Flash wirelessly

### User has issues:
- ✅ Check TROUBLESHOOTING in RELEASE_NOTES.md
- ✅ Read FLASH_INSTRUCTIONS.md again
- ✅ Check device with serial monitor

---

## 🔐 File Integrity

### Recommended: Checksums (MD5/SHA256)

In RELEASE_NOTES.md, include optional checksums:
```
bootloader.bin:  abc123def456...
partitions.bin:  xyz789abc123...
firmware.bin:    12345abcde...
```

Users can verify with:
```bash
md5sum bootloader.bin
# Compare with checksum in release notes
```

---

## 📞 Support Structure

**When users encounter issues:**

1. **Issue:** "How do I flash?"
   - ✅ Refer to: FLASH_INSTRUCTIONS.md

2. **Issue:** "My device won't boot"
   - ✅ Refer to: RELEASE_NOTES.md Troubleshooting section

3. **Issue:** "Can I update wirelessly?"
   - ✅ Refer to: OTA_INSTRUCTIONS.md

4. **Issue:** "Are there other options?"
   - ✅ Refer to: README.md in firmware_release/

**All answers are in your release documentation!**

---

## 🎉 You're All Set!

Your release package structure is ready:

1. ✅ Binary files created automatically
2. ✅ Documentation ready
3. ✅ GitHub release process defined
4. ✅ Users have everything needed
5. ✅ Support materials included

**Ready to release on GitHub!** 🚀

See GITHUB_RELEASE_GUIDE.md for step-by-step GitHub release creation.
