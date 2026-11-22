// MCLA - Guest Memory Helpers
// Provides safe access to Xbox 360 guest memory from hook implementations
#pragma once

#include <rex/runtime.h>
#include <cstdint>
#include <cstring>

namespace mcla {

// Get the guest virtual memory base pointer
inline uint8_t* GetMembase() {
    auto* runtime = rex::Runtime::instance();
    if (!runtime) return nullptr;
    return runtime->virtual_membase();
}

// Check if a guest address is a valid pointer (in the XEX image or heap range)
inline bool IsGuestPtr(uint32_t ea) {
    return ea >= 0x82000000u && ea < 0xC0000000u;
}

// Read 32-bit big-endian value from guest memory
inline uint32_t ReadGuestBE32(uint32_t guest_addr) {
    auto base = GetMembase();
    if (!base) return 0;
    auto p = base + guest_addr;
    return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
           (uint32_t(p[2]) << 8) | uint32_t(p[3]);
}

// Read 16-bit big-endian value from guest memory
inline uint16_t ReadGuestBE16(uint32_t guest_addr) {
    auto base = GetMembase();
    if (!base) return 0;
    auto p = base + guest_addr;
    return static_cast<uint16_t>((uint32_t(p[0]) << 8) | uint32_t(p[1]));
}

// Read 8-bit value from guest memory
inline uint8_t ReadGuestU8(uint32_t guest_addr) {
    auto base = GetMembase();
    if (!base) return 0;
    return base[guest_addr];
}

// Read 32-bit float from guest memory (big-endian)
inline float ReadGuestBEFloat(uint32_t guest_addr) {
    auto base = GetMembase();
    if (!base) return 0.0f;
    auto p = base + guest_addr;
    uint32_t bits = (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
                    (uint32_t(p[2]) << 8) | uint32_t(p[3]);
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}

// Write 32-bit big-endian value to guest memory
inline void WriteGuestBE32(uint32_t guest_addr, uint32_t value) {
    auto base = GetMembase();
    if (!base) return;
    auto p = base + guest_addr;
    p[0] = static_cast<uint8_t>(value >> 24);
    p[1] = static_cast<uint8_t>(value >> 16);
    p[2] = static_cast<uint8_t>(value >> 8);
    p[3] = static_cast<uint8_t>(value);
}

// Write 16-bit big-endian value to guest memory
inline void WriteGuestBE16(uint32_t guest_addr, uint16_t value) {
    auto base = GetMembase();
    if (!base) return;
    auto p = base + guest_addr;
    p[0] = static_cast<uint8_t>(value >> 8);
    p[1] = static_cast<uint8_t>(value);
}

// Write 8-bit value to guest memory
inline void WriteGuestU8(uint32_t guest_addr, uint8_t value) {
    auto base = GetMembase();
    if (!base) return;
    base[guest_addr] = value;
}

// Write 32-bit float to guest memory (big-endian)
inline void WriteGuestBEFloat(uint32_t guest_addr, float value) {
    auto base = GetMembase();
    if (!base) return;
    auto p = base + guest_addr;
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    p[0] = static_cast<uint8_t>(bits >> 24);
    p[1] = static_cast<uint8_t>(bits >> 16);
    p[2] = static_cast<uint8_t>(bits >> 8);
    p[3] = static_cast<uint8_t>(bits);
}

} // namespace mcla