#!/bin/bash
# Test: create a minimal XEX2-like file and see if ReXGlue loads it
cd /home/jhon/xbox-recomp/m2-codegen

# Create a minimal valid XEX2 header file
python3 -c "
import struct
# Minimal XEX2 header: magic 'XEX2', then some fields
data = b'XEX2' + struct.pack('<I', 0x00000001) + struct.pack('<I', 0x00003000) + b'\x00' * 100
with open('assets/test_minimal.xex', 'wb') as f:
    f.write(data)
print('Created test_minimal.xex:', len(data), 'bytes')
print('First 16 bytes:', data[:16].hex())
"
echo "---"
ls -la assets/test_minimal.xex