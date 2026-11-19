#!/usr/bin/env python3
"""Debug: verify the exact bytes ReXGlue sees when loading the XEX."""
import mmap
import struct
import os

path = "assets/default.xex"
size = os.path.getsize(path)
print(f"File: {path}, size: {size}")

with open(path, "rb") as f:
    # Read first 16 bytes directly
    f.seek(0)
    raw = f.read(16)
    print(f"Direct read first 16: {raw.hex()}")

    # Try mmap
    f.seek(0)
    fd = f.fileno()
    mm = mmap.mmap(fd, 16, access=mmap.ACCESS_READ)
    print(f"Mmap first 16: {mm[:16].hex()}")
    print(f"Mmap as uint32 LE: {[hex(struct.unpack('<I', mm[i:i+4])[0]) for i in range(0, 16, 4)]}")
    mm.close()

    # Read with os.pread
    raw_pread = os.pread(fd, 16, 0)
    print(f"os.pread first 16: {raw_pread.hex()}")

# Also check: is the file a symlink, or does it have holes?
print(f"Is symlink: {os.path.islink(path)}")
print(f"Is regular file: {os.path.isfile(path)}")

# Check for sparse file
with open(path, "rb") as f:
    f.seek(0, os.SEEK_END)
    actual_size = f.tell()
    print(f"File size via seek: {actual_size}")
    # Check if file has holes (sparse)
    stat = os.stat(path)
    print(f"st_size: {stat.st_size}, st_blocks: {stat.st_blocks} (blocks*512 = {stat.st_blocks*512})")