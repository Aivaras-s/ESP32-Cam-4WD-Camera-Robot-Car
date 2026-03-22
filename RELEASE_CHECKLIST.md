# Release Preparation Checklist

Use this checklist when preparing a new release for GitHub.

---

## 📋 Pre-Release Testing

- [ ] Firmware compiles without errors
  ```bash
  platformio run -e esp32cam
  ```

- [ ] Flashing with web flasher works
  - [ ] Test on actual ESP32-CAM board
  - [ ] Use https://esptool.spacehuhn.com/
  - [ ] Verify all three files flash correctly

- [ ] Device boots successfully
  - [ ] Check LED blinks
  - [ ] Check serial output for errors
  - [ ] Verify mDNS hostname resolves

- [ ] WiFi AP mode works
  - [ ] AP SSID visible: ESP32-CAM-Setup
  - [ ] Can connect with password: 12345678
  - [ ] Captive portal appears
  - [ ] Can configure WiFi

- [ ] Camera streaming works
  - [ ] Access http://192.168.4.1:81/stream
  - [ ] Video plays smoothly
  - [ ] Can adjust quality/size

- [ ] Motor control works
  - [ ] Joystick responds
  - [ ] All 4 motors turn
  - [ ] Motor watchdog stops motors after 1s
  - [ ] Speed levels work (Low/Medium/High)

- [ ] WebSocket control works
  - [ ] UDP control responds
  - [ ] Python scripts work if included

---

## 📁 Release Files

### Binary Files
- [ ] `bootloader.bin` (17 KB) - present in `firmware_release/`
- [ ] `partitions.bin` (3 KB) - present in `firmware_release/`
- [ ] `firmware.bin` (988 KB) - present in `firmware_release/`
- [ ] All three files compile via `extra_script.py` (check build output)

### Documentation Files
- [ ] `FLASH_INSTRUCTIONS.md` - up-to-date with all 4 methods
  - [ ] Option 1: esptool.py
  - [ ] Option 2: ESP32 Flash Download Tool
  - [ ] Option 3: PlatformIO
  - [ ] Option 4: Web-based esptool.spacehuhn.com (primary)

- [ ] `RELEASE_NOTES.md` - created and complete
  - [ ] Quick Start section
  - [ ] Features list
  - [ ] Troubleshooting guide
  - [ ] Technical specs
  - [ ] Default credentials
  - [ ] Safety warnings

- [ ] `OTA_INSTRUCTIONS.md` - OTA update procedures
- [ ] `README.md` - Feature overview included
- [ ] This file for future releases

---

## 📝 Documentation Updates

### CHANGELOG.md
- [ ] Updated with new version section
- [ ] Lists all new features
- [ ] Lists all bug fixes
- [ ] Lists breaking changes (if any)
- [ ] Dated with current date

### Release Notes
- [ ] Version number is clear (v1.0, v1.0.1, etc.)
- [ ] Release date included
- [ ] Key features highlighted with emojis
- [ ] Screenshots or diagrams (if applicable)
- [ ] Known issues documented
- [ ] Upgrade instructions clear

### Security
- [ ] Default passwords documented
- [ ] Security warnings included in release notes
- [ ] OAuth/authentication notes (if applicable)
- [ ] Deprecation warnings (if any)

---

## 🔒 Security & Legal

- [ ] No hardcoded passwords/credentials in binaries
- [ ] Sensitive information removed from documentation
- [ ] License information included
- [ ] Attribution/credits included
- [ ] Copyright notice present
- [ ] Terms of use clear

---

## 🏷️ Version & Tagging

- [ ] Version number decided (semantic versioning)
  - [ ] Major change → v2.0
  - [ ] Feature addition → v1.1
  - [ ] Bugfix only → v1.0.1

- [ ] Git tag created locally
  ```bash
  git tag v1.0
  ```

- [ ] Git tag pushed to GitHub
  ```bash
  git push origin v1.0
  ```

- [ ] Release notes mention version clearly

---

## 🌐 GitHub Release Page Setup

- [ ] Release tag exists: `v1.0` ✓
- [ ] Release title is clear: "ESP32-CAM 4WD Robot Firmware v1.0"
- [ ] Release description includes:
  - [ ] Quick start guide
  - [ ] Key features
  - [ ] Download instructions
  - [ ] First-time setup steps
  - [ ] Links to documentation
  - [ ] Known issues/limitations

- [ ] All files uploaded as release assets:
  - [ ] bootloader.bin
  - [ ] partitions.bin
  - [ ] firmware.bin
  - [ ] FLASH_INSTRUCTIONS.md
  - [ ] RELEASE_NOTES.md
  - [ ] OTA_INSTRUCTIONS.md

- [ ] Source code archived (optional)
  - [ ] Create .zip of source
  - [ ] Upload as release asset
  - [ ] See on releases page

---

## 🧪 Pre-Publish Testing

### On Windows
- [ ] Tested flasher on Windows with Chrome
- [ ] Tested with Microsoft Edge
- [ ] USB driver installation not needed (auto-detect)
- [ ] Files download correctly

### On macOS
- [ ] Tested flasher on macOS with Chrome
- [ ] Tested with Safari (if supported)
- [ ] All downloads work

### On Linux
- [ ] Tested flasher on Linux with Chrome
- [ ] Serial port detection works
- [ ] No permission issues

### Download Test
- [ ] All .bin files download successfully
- [ ] Checksums match (optional but recommended)
- [ ] File sizes are correct:
  - bootloader: ~17 KB
  - partitions: ~3 KB
  - firmware: ~988 KB (may vary)

---

## 📢 Communications

- [ ] Release announcement prepared
- [ ] GitHub Releases page filled out
- [ ] Email notification ready (if applicable)
- [ ] Forum post/blog updated (if applicable)
- [ ] Social media posts ready (if applicable)

---

## ✅ Final Quality Checks

- [ ] README clearly points to release page
- [ ] Documentation links point to latest version
- [ ] No typos in release notes
- [ ] No broken links in documentation
- [ ] File paths are consistent
- [ ] Instructions are clear and tested
- [ ] All images/diagrams display correctly

---

## 🚀 Publication Checklist

Before hitting "Publish":

- [ ] All files uploaded
- [ ] Release notes reviewed
- [ ] Version number correct
- [ ] No sensitive information exposed
- [ ] Instructions tested
- [ ] Links verified
- [ ] Emergency rollback plan understood (if needed)
- [ ] Ready for public use

---

## 📊 Post-Release

### Immediately After Publishing

- [ ] Visit release page and verify it looks correct
  ```
  https://github.com/YOUR_USERNAME/REPO/releases
  ```

- [ ] Download each file and verify:
  - [ ] bootloader.bin
  - [ ] partitions.bin
  - [ ] firmware.bin

- [ ] Test the web flasher with downloaded files
  ```
  https://esptool.spacehuhn.com/
  ```

- [ ] Verify all documentation links work

### First 24 Hours

- [ ] Monitor for user feedback/issues
- [ ] Check if any bugs are reported
- [ ] Respond to questions
- [ ] Verify flash works for different users
- [ ] Check community feedback

### Ongoing

- [ ] Keep release notes updated with known issues
- [ ] Address critical bugs with patch releases (v1.0.1)
- [ ] Plan next feature release
- [ ] Update CHANGELOG.md
- [ ] Remove release if critical bugs found (before widespread adoption)

---

## 🔄 Release Artifact Maintenance

### Keep in firmware_release/

- [ ] Always maintain latest binaries in `firmware_release/` folder
- [ ] Update after each build:
  ```bash
  # Automatic via extra_script.py
  platformio run -e esp32cam
  ```

- [ ] Don't commit .bin files to source control (optional)
  - [ ] .gitignore includes *.bin (if desired)
  - [ ] Or commit binaries for release automation

### Backup Strategy

- [ ] Old release files stored on GitHub (automatic)
- [ ] Can roll back to any previous version
- [ ] Tags mark each version

---

## 📋 Template for Release Notes

```markdown
# Version X.X.X - Release Date

## 🎯 What's New
- Feature 1
- Feature 2
- Bug fix 1

## 🚀 Quick Start
[Instructions]

## 📚 Documentation
- FLASH_INSTRUCTIONS.md
- OTA_INSTRUCTIONS.md

## ⚠️ Important
[Security/Safety notes]

## 📥 Downloads
[Links to binary files]
```

---

## ❓ Common Questions

**Q: Should I include source code in releases?**
- A: Recommended - create a .zip archive and upload as separate asset

**Q: Can I delete a release?**
- A: Yes, on GitHub releases page - but keep at least one version available

**Q: How to handle bugfix releases?**
- A: Use v1.0.1 for hotfixes, keep v1.0 available for reference

**Q: What if I need to update a release?**
- A: Create a new version (v1.0.1) instead of modifying existing release

**Q: How to test before publishing?**
- A: Use "Save as draft" option, test all links, then publish

---

## 🎉 You're Ready!

Once all items are checked, your release is ready for publishing on GitHub!

**Steps:**
1. ✅ All checks complete
2. ✅ Files ready and uploaded
3. ✅ Release notes finished
4. ✅ Testing done
5. 🎉 Click "Publish release"

**Your users can now download and use your firmware!**
