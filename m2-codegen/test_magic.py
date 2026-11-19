#!/usr/bin/env python3
"""Test: verify magic detection logic matches ReXGlue's expectation."""
import struct

# ReXGlue uses be<memory::fourcc_t> which reads as big-endian
# Then compares to kXEX2Signature = make_fourcc("XEX2") = 0x58455832 (BE)
# kXEX1Signature = make_fourcc("XEX1") = 0x58453132 (BE)

def make_fourcc(a, b, c, d):
    return (ord(a) << 24) | (ord(b) << 16) | (ord(c) << 8) | ord(d)

kXEX2 = make_fourcc('X', 'E', 'X', '2')
kXEX1 = make_fourcc('X', 'E', 'X', '1')
print(f"kXEX2Signature = 0x{kXEX2:08X}")
print(f"kXEX1Signature = 0x{kXEX1:08X}")

# Read first 4 bytes from the actual XEX
with open('assets/default.xex', 'rb') as f:
    raw = f.read(4)
    magic_be = struct.unpack('>I', raw)[0]
    magic_le = struct.unpack('<I', raw)[0]
    print(f"\nActual file first 4 bytes: {raw.hex()}")
    print(f"  As big-endian: 0x{magic_be:08X}")
    print(f"  As little-endian: 0x{magic_le:08X}")
    print(f"  Matches kXEX2 (BE): {magic_be == kXEX2}")
    print(f"  Matches kXEX2 (LE): {magic_le == kXEX2}")

# The error message said: Unknown module magic: 58327A5D
# Let's see what that would be
error_magic = 0x58327A5D
error_bytes_be = struct.pack('>I', error_magic)
error_bytes_le = struct.pack('<I', error_magic)
print(f"\nError magic 0x{error_magic:08X}:")
print(f"  As BE bytes: {error_bytes_be.hex()} = {error_bytes_be}")
print(f"  As LE bytes: {error_bytes_le.hex()} = {error_bytes_le}")