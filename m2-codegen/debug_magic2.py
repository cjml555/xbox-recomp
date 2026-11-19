#!/usr/bin/env python3
"""Test: understand what ReXGlue's be<> type does with magic comparison."""

# ReXGlue code:
# be<memory::fourcc_t> magic;
# magic.value = memory::load<memory::fourcc_t>(addr);
# if (magic == runtime::kXEX2Signature ...)

# memory::load does a simple memcpy:
# template <typename T>
# inline T load(const void* mem) {
#   T result;
#   std::memcpy(&result, mem, sizeof(T));
#   return result;
# }

# so memory::load<memory::fourcc_t>(addr) copies the 4 bytes as-is into a fourcc_t (uint32_t)

# be<T> is defined as:
# using be = endian_store<T, std::endian::big>;

# Let's understand what values we're dealing with:

import struct

# File magic: 'XEX2' stored as big-endian bytes
file_magic_be = b'XEX2'
file_magic_be_value = struct.unpack('>I', file_magic_be)[0]
file_magic_le_value = struct.unpack('<I', file_magic_be)[0]

print(f"File magic bytes (BE): {file_magic_be.hex()}")
print(f"  As uint32 BE: 0x{file_magic_be_value:08X}")
print(f"  As uint32 LE: 0x{file_magic_le_value:08X}")

# From codegen_context.cpp line 49:
# auto vfsPath = "game:\\" + xexFilename.string();
# This becomes "game:\\default.xex"

# The actual first 16 bytes of the file:
with open('m2-codegen/assets/default.xex', 'rb') as f:
    first_16 = f.read(16)

print(f"\nFirst 16 bytes of XEX file: {first_16.hex()}")

# XEX header structure (from xex2_header):
# be<uint32_t> magic;                  // 0x0 'XEX2'
# be<xex2_module_flags> module_flags;  // 0x4
# be<uint32_t> header_size;            // 0x8
# ...

header_magic = first_16[0:4]
header_magic_be = struct.unpack('>I', header_magic)[0]
header_magic_le = struct.unpack('<I', header_magic)[0]

print(f"\nHeader (offset 0x0): {header_magic.hex()}")
print(f"  BE interpretation: 0x{header_magic_be:08X}")
print(f"  LE interpretation: 0x{header_magic_le:08X}")

# kXEX2Signature = make_fourcc("XEX2")
# make_fourcc('X', 'E', 'X', '2') = (0x58 << 24) | (0x45 << 16) | (0x58 << 8) | 0x32
kXEX2Signature = (0x58 << 24) | (0x45 << 16) | (0x58 << 8) | 0x32
print(f"\nkXEX2Signature = 0x{kXEX2Signature:08X}")

# The error was: Unknown module magic: 58327A5D
# 0x58327A5D in ASCII BE: X2z]
# This doesn't look like XEX2 at all!

# Let's check what 0x58327A5D would decode to:
error_magic = 0x58327A5D
print(f"\nError magic: 0x{error_magic:08X}")
print(f"  As BE bytes: {struct.pack('>I', error_magic).hex()} = {struct.pack('>I', error_magic)}")
print(f"  As LE bytes: {struct.pack('<I', error_magic).hex()} = {struct.pack('<I', error_magic)}")

# Something is reading the wrong bytes!
# Could it be reading at the wrong offset?

# Let's check offset 0x18 onwards:
header_size_be = struct.unpack('>I', first_16[8:12])[0]
module_flags_be = struct.unpack('>I', first_16[4:8])[0]
print(f"\nHeader size (BE, offset 0x8): {header_size_be}")
print(f"Module flags (BE, offset 0x4): {module_flags_be:#010x}")

# Could the magic be coming from somewhere else?
# Or is there a problem with how be<> compares?

print("\n" + "="*60)
print("CONCLUSION:")
print("The file header starts with 'XEX2' (0x32584558 LE / 0x58455832 BE)")
print("But ReXGlue reports magic 0x58327A5D which is INVALID")
print("This suggests either:")
print("1. The file content differs from what we see")
print("2. There's a bug in the magic comparison code")
print("3. The file is being read incorrectly (wrong offset)")
print("="*60)