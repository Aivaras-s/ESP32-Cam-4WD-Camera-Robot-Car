"""
PlatformIO Extra Script
Automatically copies built firmware binaries to firmware_release folder
after successful build
"""

import os
import shutil
from pathlib import Path

Import("env")

def copy_firmware_to_release(source, target, env):
    """Copy built binaries to firmware_release folder"""
    
    build_dir = Path(env.subst("$BUILD_DIR"))
    release_dir = Path(env.subst("$PROJECT_DIR")) / "firmware_release"
    
    # Ensure release directory exists
    release_dir.mkdir(parents=True, exist_ok=True)
    
    files_to_copy = [
        "bootloader.bin",
        "firmware.bin", 
        "partitions.bin"
    ]
    
    print("\n" + "="*50)
    print("📦 Copying firmware to firmware_release folder")
    print("="*50)
    
    for file in files_to_copy:
        src = build_dir / file
        dst = release_dir / file
        
        if src.exists():
            try:
                shutil.copy2(src, dst)
                print(f"✓ {file}")
            except Exception as e:
                print(f"✗ Failed to copy {file}: {e}")
        else:
            print(f"⊘ {file} not found in build directory")
    
    print("="*50 + "\n")

# Register the post-build action
env.AddPostAction("$BUILD_DIR/firmware.bin", copy_firmware_to_release)
