// MCLA - Timer Precision Hooks
// Ported from larecomp-ref/src/mc_engine/hooks/timer_precision.cpp (BadassBaboon).
//
// The engine timer (sub_821BDA90, object at 0x827D7500) adds the frame time to two
// float clocks every frame: [+0x14] (all frames) and [+0x18] (frames the game is
// running). A float holds ~7 digits, so the sum is rounded to its own step each frame
// (0.24 ms near one hour, 3.9 ms near ten) and the clock runs a few percent fast or
// slow until it stops (~36 h at 240 FPS). Each hook keeps an exact double sum next to
// the float and stores the sum rounded to a float, so the error stops growing.
//
// Same shape: the audio engine's clock (sub_821E63F0, float at 0x8286CF34) adds
// [+0x60] of the timer every frame and hands the result to the audio engine in ms.
// And the race/time-trial/capture-the-flag clock (sub_82256220, whole ms at [+0xEA4])
// drops the fraction of every frame; the carry hook keeps that fraction for next time.

#include "guest_memory.h"
#include <SDL3/SDL.h>
#include <rex/runtime.h>
#include <cmath>
#include <cstdint>

namespace {

// A clock the guest advances as `clock = (float)(clock + inc)` once a frame. Remembers
// the exact sum next to the float and, after the guest's own add, puts back the sum
// rounded to a float.
struct FloatClock {
    double sum = 0.0;         // exact running total
    float published = 0.0f;   // what we left in the guest's clock last time
    bool valid = false;       // `sum` is the total of the guest's clock

    // `guest` is what the guest's add just stored, `inc` what it added. Returns the value
    // to store instead.
    float Advance(float guest, float inc) {
        if (!std::isfinite(guest) || !std::isfinite(inc)) {
            valid = false;
            return guest;
        }
        // The guest read back `published` and added `inc`, so it stored exactly this. If
        // it did not, something else wrote the clock (a reset, a load): start again.
        if (valid && guest != published + inc) valid = false;
        if (valid) {
            sum += static_cast<double>(inc);
        } else {
            sum = static_cast<double>(guest);
            valid = true;
        }
        published = static_cast<float>(sum);
        return published;
    }
};

FloatClock g_total_time;   // timer [+0x14]
FloatClock g_game_time;    // timer [+0x18]
FloatClock g_audio_time;   // audio engine clock at 0x8286CF34

void AdvanceTimerField(rex::ppc::Register& r3, rex::ppc::Register& f0, FloatClock& clock, uint32_t field) {
    uint8_t* base = mcla::GetMembase();
    if (!base) return;
    const uint32_t addr = static_cast<uint32_t>(r3.u64) + field;
    const float guest = mcla::ReadGuestBEFloat(addr);
    const float out = clock.Advance(guest, static_cast<float>(f0.f64));
    if (out != guest) mcla::WriteGuestBEFloat(addr, out);
}

}  // namespace

// After each `stfs fN,20(r3)` / `stfs fN,24(r3)` in sub_821BDA90 (four of each, one per
// path through the update): f0 is the frame time the path just added.
void MCLATimeAccumTotal(rex::ppc::Register& r3, rex::ppc::Register& f0) {
    AdvanceTimerField(r3, f0, g_total_time, 0x14);
    // World-clock rate diagnostic: the [+0x14] field is the engine's "now" the
    // simulation reads. If it advances ~1000 ms per host second the world is in real
    // time; much less means the game thread is starved (frame deltas huge, capped).
    static uint64_t last_host_ms = 0;
    static double last_clock_s = 0.0;
    const uint64_t host_ms = rex::chrono::Clock::QueryHostUptimeMillis();
    const double clock_s = static_cast<double>(g_total_time.published);
    if (last_host_ms && (host_ms - last_host_ms) >= 1000) {
        const double world_ms = (clock_s - last_clock_s) * 1000.0;
        const double elapsed_host_ms = double(host_ms - last_host_ms);
        const double world_ms_per_host_second = world_ms * 1000.0 / elapsed_host_ms;
        SDL_Log("[MCLA_Timing] world clock advance = %.0f ms per 1000 host ms (%.0f%%)",
                world_ms_per_host_second, 100.0 * world_ms / elapsed_host_ms);
        last_host_ms = host_ms;
        last_clock_s = clock_s;
    } else if (!last_host_ms) {
        last_host_ms = host_ms;
        last_clock_s = clock_s;
    }
}

void MCLATimeAccumGame(rex::ppc::Register& r3, rex::ppc::Register& f0) {
    AdvanceTimerField(r3, f0, g_game_time, 0x18);
}

// After `stfs f0,-12492(r9)` in sub_821E63F0: r9 is the high half of the clock's address,
// f0 the new sum (which the next instructions turn into milliseconds, so it is replaced
// too), r29 the engine timer, whose [+0x60] was just added.
void MCLAAudioClockAccum(rex::ppc::Register& r9, rex::ppc::Register& r29, rex::ppc::Register& f0) {
    uint8_t* base = mcla::GetMembase();
    if (!base) return;
    const uint32_t clock_addr = static_cast<uint32_t>(r9.u64) + static_cast<uint32_t>(-12492);
    const float inc = mcla::ReadGuestBEFloat(static_cast<uint32_t>(r29.u64) + 0x60);
    const float guest = static_cast<float>(f0.f64);
    const float out = g_audio_time.Advance(guest, inc);
    if (out != guest) {
        mcla::WriteGuestBEFloat(clock_addr, out);
        f0.f64 = static_cast<double>(out);
    }
}

// Before `fctiwz f13,f0` in sub_82256220: f0 is the frame time in milliseconds, which the
// conversion would cut to a whole number. Hands it the whole part of this frame's time and
// what was left over by the frames before, and keeps the rest for the next one.
void MCLAGameClockCarry(rex::ppc::Register& f0) {
    static double carry = 0.0;
    const double ms = f0.f64 + carry;
    if (!std::isfinite(ms) || std::fabs(ms) > 1.0e9) {
        carry = 0.0;
        return;
    }
    const double whole = std::trunc(ms);
    carry = ms - whole;
    f0.f64 = whole;
}