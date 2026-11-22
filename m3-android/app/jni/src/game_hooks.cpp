// MCLA - Game Hook Stubs Implementation
// Generated for M3 milestone
// Stub implementations that allow the app to run on Android
// Real implementations ported from LARecomp/mcla-recomp reference source

#include "game_hooks.h"
#include "config.h"
#include "guest_memory.h"
#include <SDL3/SDL.h>
#include <rex/runtime.h>
#include <rex/chrono/clock.h>
#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <string>

// Logs each stub hook name at most once. Stub hooks are called from the guest
// hot path (sometimes every frame); a plain SDL_Log here floods logcat and
// hides real diagnostics. This keeps a one-line "stub installed" trace per hook.
static void LogStubOnce(const char* name) {
    static std::mutex m;
    static std::unordered_set<std::string> seen;
    std::lock_guard<std::mutex> lk(m);
    if (seen.emplace(name).second) {
        SDL_Log("[Stub] %s (no-op on Android)", name);
    }
}

namespace mcla {

// Forward declarations for internal helper functions
uint8_t* GuestBase() {
    auto* runtime = rex::Runtime::instance();
    if (!runtime) return nullptr;
    return runtime->virtual_membase();
}

} // namespace mcla

bool Hook_BrowserCreateGameVeto(rex::ppc::Register& r27) {
    (void)r27;
    LogStubOnce("Hook_BrowserCreateGameVeto");
    return false;
}

bool Hook_CarbonPartPop(rex::ppc::Register& r1, rex::ppc::Register& r27, rex::ppc::Register& r31) {
    // r1 = stack pointer, r27 = slot, r31 = vehicle pointer
    // Carbon parts pop - restores forced technique group
    uint32_t sp = r1.u32;
    int slot = static_cast<int32_t>(r27.u64);
    uint32_t veh = r31.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[CarbonPartPop] Carbon parts pop hook installed");
    });
    
    // Simplified: return false to let vanilla run
    (void)sp;
    (void)slot;
    (void)veh;
    return false;
}

bool Hook_CarbonPartPush(rex::ppc::Register& r1, rex::ppc::Register& r27, rex::ppc::Register& r31) {
    // r1 = stack pointer, r27 = slot, r31 = vehicle pointer
    // Carbon parts push - sets forced technique group
    uint32_t sp = r1.u32;
    int slot = static_cast<int32_t>(r27.u64);
    uint32_t veh = r31.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[CarbonPartPush] Carbon parts push hook installed");
    });
    
    (void)sp;
    (void)slot;
    (void)veh;
    return false;
}

bool Hook_CarbonToggle(rex::ppc::Register& r11) {
    // r11 = CustomData + 0x40 pointer
    // Carbon parts toggle - cycles through preset masks
    uint32_t refl = r11.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[CarbonToggle] Carbon parts toggle hook installed");
    });
    
    // Simplified: return false to let vanilla toggle
    (void)refl;
    return false;
}

bool Hook_ControlCaptureKey(rex::ppc::Register& r11, rex::ppc::Register& r28) {
    // r11 = output scancode, r28 = output type pointer
    // Keyboard native control capture - simplified for Android touch
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[ControlCaptureKey] Keyboard capture hook installed");
    });
    
    // On Android, touch controls are handled by the Java side
    // Return false to let vanilla handle pad input
    return false;
}

bool Hook_ControlLabel(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    // r3 = type (0=keyboard), r4 = key/scancode, r5 = output label pointer
    // Custom control label display - simplified for Android
    uint32_t type = r3.u32;
    uint32_t key = r4.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[ControlLabel] Custom label hook installed");
    });
    
    // Return false to let vanilla handle pad/axis labels
    return false;
}

bool Hook_ControllerMenuKey(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    // r3 = screen pointer, r4 = key code
    // Controller menu key handling - toggles button prompts (Xbox/PlayStation)
    uint32_t screen = r3.u32;
    uint32_t key = r4.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[ControllerMenuKey] Controller menu key hook installed");
    });
    
    // Keys 55 and 63 are the menu navigation keys
    // Return false to let vanilla handle other keys
    if (key != 55 && key != 63) return false;
    
    // For Android, we just pass through
    return false;
}

bool Hook_EnablePMSave(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    // r3 = pause menu save pointer, r4 = output
    // Adds "Save (Dev Only)" menu item to pause menu
    uint32_t pm_save = r3.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[EnablePMSave] Save menu hook installed");
    });
    
    // Simplified: just signal success
    r4.u64 = 1;
    return true;
}

bool Hook_EnablePMTeste(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Hook_EnablePMTeste");
    return false;
}

bool Hook_FlashCommandLog(rex::ppc::Register& r5) {
    (void)r5;
    LogStubOnce("Hook_FlashCommandLog");
    return false;
}

bool Hook_LanAccept(rex::ppc::Register& r30, rex::ppc::Register& r28) {
    (void)r30;
    (void)r28;
    LogStubOnce("Hook_LanAccept");
    return false;
}

bool Hook_ListViewPopulate(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ListViewPopulate");
    return false;
}

bool Hook_MenuCameraPick(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_MenuCameraPick");
    return false;
}

bool Hook_MenuKeyDispatch(rex::ppc::Register& r31, rex::ppc::Register& r28, rex::ppc::Register& r27) {
    (void)r31;
    (void)r28;
    (void)r27;
    LogStubOnce("Hook_MenuKeyDispatch");
    return false;
}

bool Hook_PMLodTrafficClick(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    LogStubOnce("Hook_PMLodTrafficClick");
    return false;
}

bool Hook_PopulateRedirect(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_PopulateRedirect");
    return false;
}

bool Hook_PumpWake(rex::ppc::Register& r3) {
    const uint32_t ms = r3.u32;
    const int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    // Simple frame pacing: if ms == 0, let vanilla run
    if (ms == 0) return false;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[PumpWake] Frame pacing enabled");
    });
    
    // For Android, we use a simple sleep instead of Win32 events
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return true;
}

bool Hook_PumpWakeRingWait(rex::ppc::Register& r31, rex::ppc::Register& r30, rex::ppc::Register& r29, rex::ppc::Register& r26, rex::ppc::Register& r3) {
    (void)r31;
    (void)r30;
    (void)r29;
    (void)r26;
    (void)r3;
    LogStubOnce("Hook_PumpWakeRingWait");
    return false;
}

bool Hook_RexGlueCancel(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("Hook_RexGlueCancel");
    return false;
}

bool Hook_RmrDownloadFile(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r9) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    (void)r9;
    LogStubOnce("Hook_RmrDownloadFile");
    return false;
}

bool Hook_RmrGetTableSize(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    LogStubOnce("Hook_RmrGetTableSize");
    return false;
}

bool Hook_RmrOverwriteFile(rex::ppc::Register& r1, rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r9, rex::ppc::Register& r10) {
    (void)r1;
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    (void)r9;
    (void)r10;
    LogStubOnce("Hook_RmrOverwriteFile");
    return false;
}

bool Hook_RmrOverwriteUserData(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r9) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    (void)r9;
    LogStubOnce("Hook_RmrOverwriteUserData");
    return false;
}

bool Hook_RmrRateRecord(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    LogStubOnce("Hook_RmrRateRecord");
    return false;
}

bool Hook_RmrReadByGamer(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r9) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    (void)r9;
    LogStubOnce("Hook_RmrReadByGamer");
    return false;
}

bool Hook_RmrReadByRank(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r9, rex::ppc::Register& r10) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    (void)r9;
    (void)r10;
    LogStubOnce("Hook_RmrReadByRank");
    return false;
}

bool Hook_RmrReadRandom(rex::ppc::Register& r1, rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r9, rex::ppc::Register& r10) {
    (void)r1;
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    (void)r9;
    (void)r10;
    LogStubOnce("Hook_RmrReadRandom");
    return false;
}

bool Hook_RmrUploadFile(rex::ppc::Register& r1, rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r9, rex::ppc::Register& r10) {
    (void)r1;
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    (void)r9;
    (void)r10;
    LogStubOnce("Hook_RmrUploadFile");
    return false;
}

bool Hook_StringTableLanguage(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Hook_StringTableLanguage");
    return false;
}

bool MCLA_CarShaderBindDone_823794C8(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_CarShaderBindDone_823794C8");
    return false;
}

bool MCLA_CarShaderBindDone_82379548(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_CarShaderBindDone_82379548");
    return false;
}

bool MCLA_CustomMusic_StartSong(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_CustomMusic_StartSong");
    return false;
}

bool MCLA_MapCursorMagnetGuard() {
    LogStubOnce("MCLA_MapCursorMagnetGuard");
    return false;
}

bool MCLA_MapCursorSweepNullGuard(rex::ppc::Register& r31, rex::ppc::Register& r30, rex::ppc::Register& r26) {
    (void)r31;
    (void)r30;
    (void)r26;
    LogStubOnce("MCLA_MapCursorSweepNullGuard");
    return false;
}

bool MCLA_MenuListNullTableGuard(rex::ppc::Register& r30) {
    (void)r30;
    LogStubOnce("MCLA_MenuListNullTableGuard");
    return false;
}

bool MCLA_PauseListVtableGuard(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_PauseListVtableGuard");
    return false;
}

bool MCLA_ShaderGroupLookup_82193AF8(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("MCLA_ShaderGroupLookup_82193AF8");
    return false;
}

bool MCLA_TrafficBoundRelease_8259AA40(rex::ppc::Register& r30) {
    (void)r30;
    LogStubOnce("MCLA_TrafficBoundRelease_8259AA40");
    return false;
}

bool MCLA_UI_SkipMissingLights(rex::ppc::Register& r3) {
    // r3 = light entity pointer. Keep vanilla behavior (do not skip) for now;
    // returning true would discard missing-light entities during UI draw.
    uint32_t entity = r3.u32;
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiScene] SkipMissingLights hook installed");
    });
    (void)entity;
    return false;
}

bool MCLAUseRealDelta() {
    // Feed the simulation the measured frame time instead of the engine's fixed
    // 30 Hz timestep (BadassBaboon's real_frame_delta=1). The measured unscaled
    // delta is already computed at [r3+0x58] and the scaled one at [r3+0x08];
    // this jump to 0x821BDC34 makes the game use them. The guest clock is a
    // verified-real-time 50 MHz source, so this is what keeps the world advancing
    // at full speed instead of a fixed 33 ms step.
    return true;
}

bool Patch_60FPS_Byte(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Patch_60FPS_Byte");
    return false;
}

bool Patch_AspectRatio_82214BB8(rex::ppc::Register& f10) {
    (void)f10;
    LogStubOnce("Patch_AspectRatio_82214BB8");
    return false;
}

bool Patch_AspectRatio_82233EB4(rex::ppc::Register& f0, rex::ppc::Register& f30) {
    (void)f0;
    (void)f30;
    LogStubOnce("Patch_AspectRatio_82233EB4");
    return false;
}

bool Patch_AspectRatio_8223E5E0(rex::ppc::Register& f13) {
    (void)f13;
    LogStubOnce("Patch_AspectRatio_8223E5E0");
    return false;
}

bool Patch_AspectRatio_822C0918(rex::ppc::Register& f0) {
    (void)f0;
    LogStubOnce("Patch_AspectRatio_822C0918");
    return false;
}

bool Patch_AspectRatio_822E5E68(rex::ppc::Register& f12, rex::ppc::Register& f30) {
    (void)f12;
    (void)f30;
    LogStubOnce("Patch_AspectRatio_822E5E68");
    return false;
}

bool Patch_BypassContentCheck(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r3;
    (void)r4;
    (void)r5;
    LogStubOnce("Patch_BypassContentCheck");
    return false;
}

bool Patch_DebugCamGate() {
    LogStubOnce("Patch_DebugCamGate");
    return false;
}

bool Patch_DisableImposterShadows(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Patch_DisableImposterShadows");
    return false;
}

bool Patch_DisableMotionBlur(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_DisableMotionBlur");
    return false;
}

bool Patch_DisableMSAA(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Patch_DisableMSAA");
    return false;
}

bool Patch_DisableRubberBanding() {
    LogStubOnce("Patch_DisableRubberBanding");
    return false;
}

bool Patch_EdramLimit(rex::ppc::Register& r3, rex::ppc::Register& r30, rex::ppc::Register& r11) {
    (void)r3;
    (void)r30;
    (void)r11;
    LogStubOnce("Patch_EdramLimit");
    return false;
}

bool Patch_FenceSpinThrottle() {
    // The guest's D3D fence poll (sub_82412F98) runs a tight loop of 32 `mr r31,r31`
    // NOPs meant to pause in-order PPC pipelines. On the host those compile to an
    // unconstrained busy-wait that burns ~40% of the render thread. Yield instead of
    // spinning: returning true jumps to 0x82412FD8, bypassing the 32 NOPs. This frees
    // CPU so the game thread isn't starved (the slow-motion culprit).
    std::this_thread::yield();
    return true;
}

bool Patch_ImpostorShadowGuard(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_ImpostorShadowGuard");
    return false;
}

bool Patch_MapVisibleAspect_82620700(rex::ppc::Register& f27) {
    (void)f27;
    LogStubOnce("Patch_MapVisibleAspect_82620700");
    return false;
}

bool Patch_PhysicsCollision() {
    LogStubOnce("Patch_PhysicsCollision");
    return false;
}

bool Patch_PlatformPush(rex::ppc::Register& r5) {
    (void)r5;
    LogStubOnce("Patch_PlatformPush");
    return false;
}

bool Patch_PropOwnerPoseGuard(rex::ppc::Register& r31, rex::ppc::Register& r4) {
    (void)r31;
    (void)r4;
    LogStubOnce("Patch_PropOwnerPoseGuard");
    return false;
}

bool Patch_PropOwnerSyncGuard(rex::ppc::Register& r31, rex::ppc::Register& r4) {
    (void)r31;
    (void)r4;
    LogStubOnce("Patch_PropOwnerSyncGuard");
    return false;
}

bool Patch_RearViewFrameSkip() {
    LogStubOnce("Patch_RearViewFrameSkip");
    return false;
}

bool Patch_SpeedUnits(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Patch_SpeedUnits");
    return false;
}

bool Patch_WheelFitBypass() {
    LogStubOnce("Patch_WheelFitBypass");
    return false;
}

bool SkipIntro() {
    LogStubOnce("SkipIntro");
    return false;
}

void Hook_BrowserJoinSession(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_BrowserJoinSession");
}

void Hook_BrowserLanResult(rex::ppc::Register& r1, rex::ppc::Register& r27) {
    (void)r1;
    (void)r27;
    LogStubOnce("Hook_BrowserLanResult");
}

void Hook_BrowserLiveResult(rex::ppc::Register& r30, rex::ppc::Register& r3) {
    (void)r30;
    (void)r3;
    LogStubOnce("Hook_BrowserLiveResult");
}

void Hook_CacheVinylPaint(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    LogStubOnce("Hook_CacheVinylPaint");
}

void Hook_CameraMatrixPublish(rex::ppc::Register& r30) {
    (void)r30;
    LogStubOnce("Hook_CameraMatrixPublish");
}

void Hook_CameraPostUpdate(rex::ppc::Register& r20) {
    (void)r20;
    LogStubOnce("Hook_CameraPostUpdate");
}

void Hook_CameraPrePublish(rex::ppc::Register& r20) {
    (void)r20;
    LogStubOnce("Hook_CameraPrePublish");
}

void Hook_CameraStateUpdate(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("Hook_CameraStateUpdate");
}

void Hook_CameraSystem(rex::ppc::Register& r20, rex::ppc::Register& r24) {
    (void)r20;
    (void)r24;
    LogStubOnce("Hook_CameraSystem");
}

void Hook_CaptureDistrict(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_CaptureDistrict");
}

void Hook_CapturePMContinue(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_CapturePMContinue");
}

void Hook_CarbonGarageProbe(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_CarbonGarageProbe");
}

void Hook_CarbonGarageSelect(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    LogStubOnce("Hook_CarbonGarageSelect");
}

void Hook_CarbonUiTick(rex::ppc::Register& r3) {
    // r3 = Carbon UI context pointer
    uint32_t ctx_ptr = r3.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[Carbon] UiTick hook installed");
    });
    
    (void)ctx_ptr;
}

void Hook_ChallengeClockGate(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ChallengeClockGate");
}

void Hook_ChallengeHostPublishGate(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Hook_ChallengeHostPublishGate");
}

void Hook_ChallengeIsGroupHost(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ChallengeIsGroupHost");
}

void Hook_ChallengeProp3Gate(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ChallengeProp3Gate");
}

void Hook_ChallengeProposeRoute(rex::ppc::Register& r3, rex::ppc::Register& r29) {
    (void)r3;
    (void)r29;
    LogStubOnce("Hook_ChallengeProposeRoute");
}

void Hook_ChallengeResolveChallenger(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ChallengeResolveChallenger");
}

void Hook_ChallengeTimeGate(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Hook_ChallengeTimeGate");
}

void Hook_ClockAmOwner(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ClockAmOwner");
}

void Hook_ClockOwnerRecord(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("Hook_ClockOwnerRecord");
}

void Hook_ClockResolveOwner(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ClockResolveOwner");
}

void Hook_ClockSessionReady(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ClockSessionReady");
}

void Hook_ClockStarting(rex::ppc::Register& r5, rex::ppc::Register& r7) {
    (void)r5;
    (void)r7;
    LogStubOnce("Hook_ClockStarting");
}

void Hook_ControlMapRebuilt(rex::ppc::Register& r30) {
    (void)r30;
    LogStubOnce("Hook_ControlMapRebuilt");
}

void Hook_DisplayGammaType(rex::ppc::Register& r1) {
    (void)r1;
    LogStubOnce("Hook_DisplayGammaType");
}

void Hook_DumpJoinBlob(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("Hook_DumpJoinBlob");
}

void Hook_InviteReceiveProbe(rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r4;
    (void)r5;
    LogStubOnce("Hook_InviteReceiveProbe");
}

void Hook_InviteSendProbe(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Hook_InviteSendProbe");
}

void Hook_KeyboardFeed() {
    LogStubOnce("Hook_KeyboardFeed");
}

void Hook_LanFillElement(rex::ppc::Register& r27, rex::ppc::Register& r28, rex::ppc::Register& r30) {
    (void)r27;
    (void)r28;
    (void)r30;
    LogStubOnce("Hook_LanFillElement");
}

void Hook_LeaderboardWrite(rex::ppc::Register& r25, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r25;
    (void)r5;
    (void)r6;
    LogStubOnce("Hook_LeaderboardWrite");
}

void Hook_LzxDecompressPre(rex::ppc::Register& r1) {
    // r1 = pointer to LZX decompress context
    uint32_t ctx = r1.u32;
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[LzxDecompress] Pre hook installed");
    });
    // Could add decompression logging or custom decompression here
}

void Hook_LzxDecompressPost(rex::ppc::Register& r1, rex::ppc::Register& r3) {
    // r1 = context, r3 = result
    uint32_t ctx = r1.u32;
    uint32_t result = r3.u32;
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[LzxDecompress] Post hook installed");
    });
    // Could verify decompression result here
}

void Hook_MapInputTick(rex::ppc::Register& r31, rex::ppc::Register& f1) {
    // r31 = map input context, f1 = delta time
    // Map mouse/pointer input processing - simplified for Android touch
    uint32_t ctx = r31.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[MapInputTick] Map input tick hook installed");
    });
    
    // Android touch input for map is handled by the Java side
    (void)ctx;
}

void Hook_MenuCameraFinal(rex::ppc::Register& r1, rex::ppc::Register& r24) {
    (void)r1;
    (void)r24;
    LogStubOnce("Hook_MenuCameraFinal");
}

void Hook_NetBigSessionDefault(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Hook_NetBigSessionDefault");
}

void Hook_NetBigSessionFlag(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Hook_NetBigSessionFlag");
}

void Hook_NetLinkWhileDown(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_NetLinkWhileDown");
}

void Hook_NetLinkWhileUp(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_NetLinkWhileUp");
}

void Hook_PadBitsFinalize(rex::ppc::Register& r31) {
    // r31 = pointer to pad state
    // Finalizes pad button states after reading - simplified for Android
    uint32_t pad_ptr = r31.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[PadBitsFinalize] Pad state finalize hook installed");
    });
    
    // Android touch/gamepad input is handled by SDL/Java side
    // This hook would normally process the raw pad bits
    (void)pad_ptr;
}

void Hook_PhotoModeCapture(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_PhotoModeCapture");
}

void Hook_PlatformVarInit(rex::ppc::Register& r27) {
    (void)r27;
    LogStubOnce("Hook_PlatformVarInit");
}

void Hook_PromptDropped() {
    LogStubOnce("Hook_PromptDropped");
}

void Hook_PromptEntryGate(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Hook_PromptEntryGate");
}

void Hook_PromptFlowGate(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_PromptFlowGate");
}

void Hook_PromptRaised() {
    LogStubOnce("Hook_PromptRaised");
}

void Hook_PromptStateIdle(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Hook_PromptStateIdle");
}

void Hook_PromptTableGate(rex::ppc::Register& r3, rex::ppc::Register& r25) {
    (void)r3;
    (void)r25;
    LogStubOnce("Hook_PromptTableGate");
}

void Hook_PromptTimeGate(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_PromptTimeGate");
}

void Hook_PropOwnerCreated(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    LogStubOnce("Hook_PropOwnerCreated");
}

void Hook_PropOwnerFreed(rex::ppc::Register& r31, rex::ppc::Register& r1) {
    (void)r31;
    (void)r1;
    LogStubOnce("Hook_PropOwnerFreed");
}

void Hook_PumpWakeMixerDone() {
    // Mixer-done callback from the audio pump wake path.
    // On Android the guest audio pump is driven from the main loop;
    // this callback only needs to acknowledge completion.
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[PumpWake] MixerDone hook installed");
    });
}

void Hook_PumpWakeSlotFreed() {
    // On Android, we don't have the Win32 event system
    // The frame pacing is handled by Hook_PumpWake sleep
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[PumpWake] SlotFreed hook called (no-op on Android)");
    });
}

void Hook_RmrEnter(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_RmrEnter");
}

void Hook_RmrPump(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_RmrPump");
}

void Hook_RoadReflCarsEnter(rex::ppc::Register& r30, rex::ppc::Register& r28, rex::ppc::Register& r13) {
    (void)r30;
    (void)r28;
    (void)r13;
    LogStubOnce("Hook_RoadReflCarsEnter");
}

void Hook_RoadReflCarsGate(rex::ppc::Register& r10) {
    (void)r10;
    LogStubOnce("Hook_RoadReflCarsGate");
}

void Hook_RoadReflCarsPlane(rex::ppc::Register& f0) {
    (void)f0;
    LogStubOnce("Hook_RoadReflCarsPlane");
}

void Hook_ScriptButtonRead(rex::ppc::Register& r3, rex::ppc::Register& r30, rex::ppc::Register& r7, rex::ppc::Register& r11) {
    // r3 = button index, r30 = pad state, r7 = output pressed, r11 = output analog
    // Script button reading - simplified for Android
    uint32_t btn_idx = r3.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[ScriptButtonRead] Script button read hook installed");
    });
    
    (void)btn_idx;
}

void Hook_ScriptButtonRefused() {
    LogStubOnce("Hook_ScriptButtonRefused");
}

void Hook_ScriptHighbeamsAnswer(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ScriptHighbeamsAnswer");
}

void Hook_ScriptHighbeamsAsk(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Hook_ScriptHighbeamsAsk");
}

void Hook_ScriptHighbeamsRefused() {
    LogStubOnce("Hook_ScriptHighbeamsRefused");
}

void Hook_ScriptPadRead(rex::ppc::Register& r8, rex::ppc::Register& r5, rex::ppc::Register& r3, rex::ppc::Register& r6) {
    // r8 = pad index, r5 = button mask, r3 = output state, r6 = output analog
    // Script pad input reading - simplified for Android
    uint32_t pad_idx = r8.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[ScriptPadRead] Script pad read hook installed");
    });
    
    // Android input handled by SDL/Java
    (void)pad_idx;
}

void Hook_ScriptUiPressedAnswer(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_ScriptUiPressedAnswer");
}

void Hook_ScriptUiPressedAsk(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Hook_ScriptUiPressedAsk");
}

void Hook_SessionCreateObject(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("Hook_SessionCreateObject");
}

void Hook_SquashInjectMatrix(rex::ppc::Register& r4, rex::ppc::Register& r6) {
    (void)r4;
    (void)r6;
    LogStubOnce("Hook_SquashInjectMatrix");
}

void Hook_SquashSelectVehicle(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_SquashSelectVehicle");
}

void Hook_SquashWorldMatrix(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("Hook_SquashWorldMatrix");
}

void Hook_SwfContextEnter(rex::ppc::Register& r3) {
    // r3 = SWF movie context pointer
    uint32_t ctx = r3.u32;
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[SWF] Context enter hook installed");
    });
    // Track current SWF context for UI hooks
}

void Hook_TimeOfDay(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_TimeOfDay");
}

void Hook_TireOffsetWheels(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("Hook_TireOffsetWheels");
}

void Hook_TournamentPump(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Hook_TournamentPump");
}

void Hook_TriggerLineSweep(rex::ppc::Register& r1, rex::ppc::Register& r31, rex::ppc::Register& r27) {
    (void)r1;
    (void)r31;
    (void)r27;
    LogStubOnce("Hook_TriggerLineSweep");
}

void Hook_TriggersHud(rex::ppc::Register& r30) {
    (void)r30;
    LogStubOnce("Hook_TriggersHud");
}

void Hook_UiCamCtrl(rex::ppc::Register& r31, rex::ppc::Register& f0) {
    (void)r31;
    (void)f0;
    LogStubOnce("Hook_UiCamCtrl");
}

void Hook_UiCamDone() {
    LogStubOnce("Hook_UiCamDone");
}

void Hook_UiColor_Edit(rex::ppc::Register& r11) {
    // r11 = ARGB color value for edit/selection UI elements
    uint32_t color = r11.u32;

    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiColor] Edit remapping hook installed");
    });

    (void)color;
}

void Hook_UiColor_Fill(rex::ppc::Register& r11) {
    // r11 = ARGB color value
    // UI color remapping hook - simplified for Android
    uint32_t color = r11.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiColor] Fill remapping hook installed");
    });
    
    // Full implementation would remap colors based on theme
    // For now, just pass through
    (void)color;
}

void Hook_UiColor_TextInit(rex::ppc::Register& r11) {
    // r11 = ARGB color value
    uint32_t color = r11.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiColor] TextInit remapping hook installed");
    });
    
    (void)color;
}

void Hook_UiColor_TextRun(rex::ppc::Register& r11) {
    // r11 = ARGB color value
    uint32_t color = r11.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiColor] TextRun remapping hook installed");
    });
    
    (void)color;
}

void Hook_UiColor_Line(rex::ppc::Register& r11) {
    // r11 = ARGB color value for line rendering
    uint32_t color = r11.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiColor] Line remapping hook installed");
    });
    
    (void)color;
}

void Hook_UiCountdownDt(rex::ppc::Register& r3, rex::ppc::Register& f1) {
    (void)r3;
    (void)f1;
    LogStubOnce("Hook_UiCountdownDt");
}

void Hook_UiSceneEnter(rex::ppc::Register& r3) {
    // r3 = scene node pointer entering the UI draw traversal
    uint32_t node = r3.u32;
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiScene] Enter hook installed");
    });
    (void)node;
}

void Hook_UiSceneFloat(rex::ppc::Register& r1, rex::ppc::Register& r30, rex::ppc::Register& r31) {
    // UI scene float parameter hook (r1/r30/r31 carry scene state)
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiScene] Float hook installed");
    });
    (void)r1;
    (void)r30;
    (void)r31;
}

void Hook_UiSceneLeave() {
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiScene] Leave hook installed");
    });
}

void Hook_UiSceneVec(rex::ppc::Register& r1, rex::ppc::Register& r30, rex::ppc::Register& r31) {
    // UI scene vector parameter hook (r1/r30/r31 carry scene state)
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiScene] Vec hook installed");
    });
    (void)r1;
    (void)r30;
    (void)r31;
}

void Hook_UiSmoothDraw(rex::ppc::Register& r30, rex::ppc::Register& r31) {
    // r30 = node instance, r31 = parent state block
    // UI frame interpolation hook - simplified for Android
    uint32_t inst = r30.u32;
    uint32_t parent_s = r31.u32;
    
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[UiSmooth] Draw interpolation hook installed");
    });
    
    // Full implementation would read guest memory and interpolate transforms
    // For now, just log that it's being called
    (void)inst;
    (void)parent_s;
}

void Hook_VibrationSet(rex::ppc::Register& r3, rex::ppc::Register& r5) {
    (void)r3;
    (void)r5;
    LogStubOnce("Hook_VibrationSet");
}

void Hook_WheelHudGear(rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r4;
    (void)r5;
    LogStubOnce("Hook_WheelHudGear");
}

void Hook_WheelSlip(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("Hook_WheelSlip");
}

void MCLAAmbientDensityTuning(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("MCLAAmbientDensityTuning");
}

void MCLA_Audio_BankError(rex::ppc::Register& r4, rex::ppc::Register& r31) {
    (void)r4;
    (void)r31;
    LogStubOnce("MCLA_Audio_BankError");
}

void MCLA_Audio_BankOpened(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    LogStubOnce("MCLA_Audio_BankOpened");
}

void MCLABikeGyroPositionFilter(rex::ppc::Register& f13) {
    (void)f13;
    LogStubOnce("MCLABikeGyroPositionFilter");
}

void MCLABikeGyroVelocityFilter(rex::ppc::Register& f0) {
    (void)f0;
    LogStubOnce("MCLABikeGyroVelocityFilter");
}

void MCLACameraBoomFilters(rex::ppc::Register& f13, rex::ppc::Register& f9, rex::ppc::Register& f1) {
    (void)f13;
    (void)f9;
    (void)f1;
    LogStubOnce("MCLACameraBoomFilters");
}

void MCLACameraLookAtSmoothing(rex::ppc::Register& f0) {
    (void)f0;
    LogStubOnce("MCLACameraLookAtSmoothing");
}

void MCLACameraPosSmoothing(rex::ppc::Register& f13) {
    (void)f13;
    LogStubOnce("MCLACameraPosSmoothing");
}

void MCLACameraShakeFilter(rex::ppc::Register& f31, rex::ppc::Register& f30, rex::ppc::Register& f29, rex::ppc::Register& f0, rex::ppc::Register& r1) {
    (void)f31;
    (void)f30;
    (void)f29;
    (void)f0;
    (void)r1;
    LogStubOnce("MCLACameraShakeFilter");
}

void MCLACameraTransitionComplete(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("MCLACameraTransitionComplete");
}

void MCLACameraTransitionSmoothing(rex::ppc::Register& r27, rex::ppc::Register& r26, rex::ppc::Register& r4, rex::ppc::Register& r1) {
    (void)r27;
    (void)r26;
    (void)r4;
    (void)r1;
    LogStubOnce("MCLACameraTransitionSmoothing");
}

void MCLA_CamEventClip(rex::ppc::Register& r30, rex::ppc::Register& r28, rex::ppc::Register& r27) {
    (void)r30;
    (void)r28;
    (void)r27;
    LogStubOnce("MCLA_CamEventClip");
}

void MCLA_CamEventEnter(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("MCLA_CamEventEnter");
}

void MCLA_CamEventNoPack() {
    LogStubOnce("MCLA_CamEventNoPack");
}

void MCLA_CamEventPool(rex::ppc::Register& r29) {
    (void)r29;
    LogStubOnce("MCLA_CamEventPool");
}

void MCLA_CamEventQueued(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("MCLA_CamEventQueued");
}

void MCLAChassisDepthSmoothing(rex::ppc::Register& f0) {
    (void)f0;
    LogStubOnce("MCLAChassisDepthSmoothing");
}

void MCLAChassisGroundPushbackDamper(rex::ppc::Register& f0, rex::ppc::Register& f3) {
    (void)f0;
    (void)f3;
    LogStubOnce("MCLAChassisGroundPushbackDamper");
}

void MCLAChassisPushbackAccel(rex::ppc::Register& f1) {
    (void)f1;
    LogStubOnce("MCLAChassisPushbackAccel");
}

void MCLA_CineActorBound(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("MCLA_CineActorBound");
}

void MCLA_CineActorClip(rex::ppc::Register& r20) {
    (void)r20;
    LogStubOnce("MCLA_CineActorClip");
}

void MCLA_CineActorEntity(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_CineActorEntity");
}

void MCLA_CineActorName(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("MCLA_CineActorName");
}

void MCLA_CineActorStart(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("MCLA_CineActorStart");
}

void MCLA_CineCamInstall(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r3;
    (void)r4;
    (void)r5;
    LogStubOnce("MCLA_CineCamInstall");
}

void MCLA_CineCamUpdate(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("MCLA_CineCamUpdate");
}

void MCLA_CineCharActor(rex::ppc::Register& r18) {
    (void)r18;
    LogStubOnce("MCLA_CineCharActor");
}

void MCLA_CineCharAnim(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    LogStubOnce("MCLA_CineCharAnim");
}

void MCLA_CineCharLoaded(rex::ppc::Register& r8) {
    (void)r8;
    LogStubOnce("MCLA_CineCharLoaded");
}

void MCLA_CineCharLoad(rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r4;
    (void)r5;
    LogStubOnce("MCLA_CineCharLoad");
}

void MCLA_CineCharQueued(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("MCLA_CineCharQueued");
}

void MCLA_CineEventFired(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("MCLA_CineEventFired");
}

void MCLA_CineEventTime(rex::ppc::Register& f1, rex::ppc::Register& r31) {
    (void)f1;
    (void)r31;
    LogStubOnce("MCLA_CineEventTime");
}

void MCLA_CineEventWalk(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_CineEventWalk");
}

void MCLA_CustomMusic_Announce(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("MCLA_CustomMusic_Announce");
}

void MCLA_CustomMusic_CueGain(rex::ppc::Register& r4, rex::ppc::Register& r6) {
    (void)r4;
    (void)r6;
    LogStubOnce("MCLA_CustomMusic_CueGain");
}

void MCLA_CustomMusic_Install(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_CustomMusic_Install");
}

void MCLA_CustomMusic_Pause(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("MCLA_CustomMusic_Pause");
}

void MCLA_CustomMusic_SelectSong(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("MCLA_CustomMusic_SelectSong");
}

void MCLA_CustomMusic_Stop(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("MCLA_CustomMusic_Stop");
}

void MCLA_DriverAnimDict(rex::ppc::Register& r28, rex::ppc::Register& r29) {
    (void)r28;
    (void)r29;
    LogStubOnce("MCLA_DriverAnimDict");
}

void MCLA_DriverAnimName(rex::ppc::Register& r1) {
    (void)r1;
    LogStubOnce("MCLA_DriverAnimName");
}

void MCLA_DriverAnimPack(rex::ppc::Register& r1) {
    (void)r1;
    LogStubOnce("MCLA_DriverAnimPack");
}

void MCLA_FileOpenResult(rex::ppc::Register& r1, rex::ppc::Register& r3) {
    (void)r1;
    (void)r3;
    LogStubOnce("MCLA_FileOpenResult");
}

void MCLAFixedStepPath(rex::ppc::Register& r3, rex::ppc::Register& f11) {
    // The other fixed-timestep path (0x821BDB90, taken when [r3+0x3A]/[r3+0x3C] is
    // set). f11 feeds `stfs f11,0x58(r3)` and `fmuls f0,f11,f13`->`stfs f0,8(r3)`;
    // replacing it with [r3+0x58] (the measured unscaled delta stored at 0x821BDAF8)
    // publishes the real frame time down this path too.
    uint8_t* base = mcla::GetMembase();
    if (!base) return;
    f11.f64 = static_cast<double>(
        mcla::ReadGuestBEFloat(static_cast<uint32_t>(r3.u64) + 0x58));
}

// Guest ticks per millisecond (the guest timebase is 50 MHz).
static double GuestTicksPerMs() {
    static const double per_ms = [] {
        uint64_t hz = rex::chrono::Clock::guest_tick_frequency();
        if (hz == 0) hz = 50000000;
        return static_cast<double>(hz) * 0.001;
    }();
    return per_ms;
}

// The step cap for one frame's delta (the hitch clamp). 125 ms was the reference's
// ceiling; a stall longer than this is played in slow motion instead of jumping
// the world forward. When the recompiler cannot sustain a short frame the world
// runs permanently slow, so the ceiling is tunable (config.json, 50..500 ms).
static double StepCapMs() {
    const double cap = static_cast<double>(mcla::GetConfig().frame_step_cap_ms);
    return cap > 0.0 ? cap : 125.0;
}

// Host-side frame limiter: holds the game thread to the fps limit (60 by default)
// by sleeping to the deadline. Mirrors the reference's EnforceFrameLimit in the
// non-spin (yield) mode so it doesn't burn a core. Called from MCLAFrameDelta on the
// game thread (first caller of sub_821BDA90 binds the owner).
static void EnforceFrameLimit() {
    constexpr int32_t kLimitFps = 60;
    static const std::thread::id owner = std::this_thread::get_id();
    if (std::this_thread::get_id() != owner) return;
    static uint64_t next_us = 0;
    const double period_us = 1000000.0 / static_cast<double>(kLimitFps);
    auto now_us = [] {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    };
    uint64_t now = now_us();
    if (next_us == 0) {
        next_us = now + static_cast<uint64_t>(period_us);
        return;
    }
    if (now < next_us) {
        const uint64_t remaining = next_us - now;
        // Sleep most of it, then yield the tail. nanosleep wakes within ~0.1 ms,
        // so sleep to 0.3 ms short and finish on yield (no CPU burn).
        constexpr uint64_t kSlackUs = 300;
        if (remaining > kSlackUs + 500) {
            std::this_thread::sleep_for(std::chrono::microseconds(remaining - kSlackUs));
        }
        while (now_us() < next_us) std::this_thread::yield();
    }
    const uint64_t after = now_us();
    next_us += static_cast<uint64_t>(period_us);
    if (next_us < after) next_us = after + static_cast<uint64_t>(period_us);
}

void MCLAFrameDelta(rex::ppc::Register& r8) {
    // The hitch clamp runs unconditionally: it is the last line of defence against an
    // unbounded delta reaching the physics and audio clocks after a streaming stall.
    const uint64_t cap = static_cast<uint64_t>(StepCapMs() * GuestTicksPerMs());
    if (r8.u64 > cap) {
        SDL_Log("[MCLA_Timing] frame step capped: %.1f ms -> %.1f ms",
                static_cast<double>(r8.u64) / GuestTicksPerMs(), StepCapMs());
        r8.u64 = cap;
    }
    // 60 FPS pacing on the game thread (with real_frame_delta the simulation is fed
    // the real measured time; the limiter keeps the update/present cadence at 60).
    EnforceFrameLimit();
}

void MCLAFrameStart() {
    // Diagnostic: measure guest-clock rate vs host wall clock once per second.
    // Called at the top of sub_821BDA90 (main frame ticker). If the guest tick
    // count advances ~50 MHz relative to real time, the guest clock is healthy
    // and slow-motion is CPU-bound (spin contention), NOT clock drift.
    static uint64_t last_host = 0;
    static uint64_t last_guest = 0;
    static bool announced = false;
    uint64_t now_host = rex::chrono::Clock::QueryHostUptimeMillis();
    uint64_t now_guest = rex::chrono::Clock::QueryGuestTickCount();
    if (!last_host) {
        last_host = now_host;
        last_guest = now_guest;
    } else if (now_host - last_host >= 1000) {
        double host_dt_ms = double(now_host - last_host);
        double guest_dt = double(now_guest - last_guest);
        double guest_freq = (guest_dt / host_dt_ms) * 1000.0;
        SDL_Log("[MCLA_Timing] guest tick rate = %.1f MHz (target 50.0); host %.1f ms",
                guest_freq / 1e6, host_dt_ms);
        last_host = now_host;
        last_guest = now_guest;
        (void)announced;
    }
}

void MCLA_GameCamActive(rex::ppc::Register& r11) {
    (void)r11;
    LogStubOnce("MCLA_GameCamActive");
}

void MCLA_GameCamInGame(rex::ppc::Register& r19) {
    (void)r19;
    LogStubOnce("MCLA_GameCamInGame");
}

void MCLA_GuestInterruptProbe(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    // Diagnostic hook polled every guest frame to inspect interrupt state.
    // No action needed on Android; log only once to avoid per-frame spam.
    static std::once_flag announced;
    std::call_once(announced, [] {
        SDL_Log("[Guest] InterruptProbe hook installed");
    });
    (void)r3;
    (void)r31;
}

void MCLA_InflateStep(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("MCLA_InflateStep");
}

void MCLA_RageFatal(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    LogStubOnce("MCLA_RageFatal");
}

void MCLA_ResourceFixupError(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r3;
    (void)r4;
    (void)r5;
    LogStubOnce("MCLA_ResourceFixupError");
}

void MCLA_ResourceSegments(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("MCLA_ResourceSegments");
}

void MCLA_SkipIntroRenderPassMask(rex::ppc::Register& r4) {
    (void)r4;
    LogStubOnce("MCLA_SkipIntroRenderPassMask");
}

void MCLA_StreamBufferProbe(rex::ppc::Register& r3) {
    // Pure observer midasm hook at sub_821BE250 entry (rage stream read).
    // Reference (LARecomp diagnostics.cpp) gates all work on the fxc_probe
    // cvar, off by default: no register writes, no return-value influence.
    // Keep it silent — per-call SDL_Log spammed logcat with hundreds of
    // lines/s on the streaming thread.
    (void)r3;
}

void MCLA_StreamOpenResult(rex::ppc::Register& r1, rex::ppc::Register& r3) {
    (void)r1;
    (void)r3;
    LogStubOnce("MCLA_StreamOpenResult");
}

void MCLATimeScaleOverride(rex::ppc::Register& f13) {
    (void)f13;
    LogStubOnce("MCLATimeScaleOverride");
}

void MCLATireSlipSmoothing(rex::ppc::Register& f0) {
    (void)f0;
    LogStubOnce("MCLATireSlipSmoothing");
}

void MCLA_TrafficChassisBound_8232D048(rex::ppc::Register& r9, rex::ppc::Register& r11) {
    (void)r9;
    (void)r11;
    LogStubOnce("MCLA_TrafficChassisBound_8232D048");
}

void MCLA_TrafficChassisBound_8232D900(rex::ppc::Register& r3, rex::ppc::Register& r11) {
    (void)r3;
    (void)r11;
    LogStubOnce("MCLA_TrafficChassisBound_8232D900");
}

void MCLA_TrafficChassisBound_8232E274(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    LogStubOnce("MCLA_TrafficChassisBound_8232E274");
}

void MCLA_TuneFieldProbe(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    LogStubOnce("MCLA_TuneFieldProbe");
}

void Patch_AmbientCascadeClient(rex::ppc::Register& r11, rex::ppc::Register& r30, rex::ppc::Register& r31) {
    (void)r11;
    (void)r30;
    (void)r31;
    LogStubOnce("Patch_AmbientCascadeClient");
}

void Patch_AmbientCascadeDraw(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r13) {
    (void)r3;
    (void)r4;
    (void)r13;
    LogStubOnce("Patch_AmbientCascadeDraw");
}

void Patch_ArchiveList(rex::ppc::Register& r1) {
    (void)r1;
    LogStubOnce("Patch_ArchiveList");
}

void Patch_BypassVehicleDLC(rex::ppc::Register& r30) {
    (void)r30;
    LogStubOnce("Patch_BypassVehicleDLC");
}

void Patch_CutsceneFovAspect_822C04E8(rex::ppc::Register& f1) {
    (void)f1;
    LogStubOnce("Patch_CutsceneFovAspect_822C04E8");
}

void Patch_DebugCam(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_DebugCam");
}

void Patch_DeltaTime(rex::ppc::Register& r24) {
    (void)r24;
    LogStubOnce("Patch_DeltaTime");
}

void Patch_DeltaTimePre() {
    LogStubOnce("Patch_DeltaTimePre");
}

void Patch_DevOptionRegistered(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_DevOptionRegistered");
}

void Patch_DofComposite(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_DofComposite");
}

void Patch_FOVScale(rex::ppc::Register& f1, rex::ppc::Register& r24) {
    (void)f1;
    (void)r24;
    LogStubOnce("Patch_FOVScale");
}

void Patch_MapVisibleHalfFov_82620708(rex::ppc::Register& f31) {
    (void)f31;
    LogStubOnce("Patch_MapVisibleHalfFov_82620708");
}

void Patch_RearViewCarPhase(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    LogStubOnce("Patch_RearViewCarPhase");
}

void Patch_RearViewCullBits(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_RearViewCullBits");
}

void Patch_RearViewDiag() {
    LogStubOnce("Patch_RearViewDiag");
}

void Patch_RearViewDrawAlpha(rex::ppc::Register& f13) {
    (void)f13;
    LogStubOnce("Patch_RearViewDrawAlpha");
}

void Patch_RearViewUpdateActive(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_RearViewUpdateActive");
}

void Patch_RearViewUpdateVisible(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_RearViewUpdateVisible");
}

void Patch_RideHeightFit(rex::ppc::Register& f30) {
    (void)f30;
    LogStubOnce("Patch_RideHeightFit");
}

void Patch_ScaleTrafficLOD(rex::ppc::Register& f0) {
    (void)f0;
    LogStubOnce("Patch_ScaleTrafficLOD");
}

void Patch_SingleTile(rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r17, rex::ppc::Register& r25, rex::ppc::Register& r28) {
    (void)r7;
    (void)r8;
    (void)r17;
    (void)r25;
    (void)r28;
    LogStubOnce("Patch_SingleTile");
}

void Patch_SpeedLimitUnits(rex::ppc::Register& r8) {
    (void)r8;
    LogStubOnce("Patch_SpeedLimitUnits");
}

void Patch_TrafficCascadeGate(rex::ppc::Register& r11, rex::ppc::Register& r13, rex::ppc::Register& r28, rex::ppc::Register& r30) {
    (void)r11;
    (void)r13;
    (void)r28;
    (void)r30;
    LogStubOnce("Patch_TrafficCascadeGate");
}

void Patch_UiMarkerX_82289808(rex::ppc::Register& f7) {
    (void)f7;
    LogStubOnce("Patch_UiMarkerX_82289808");
}

void Patch_UiMarkerY_82289810(rex::ppc::Register& f8) {
    (void)f8;
    LogStubOnce("Patch_UiMarkerY_82289810");
}

void Patch_UiPiece_Begin(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("Patch_UiPiece_Begin");
}

void Patch_UiPiece_End(rex::ppc::Register& r30) {
    (void)r30;
    LogStubOnce("Patch_UiPiece_End");
}

void Patch_UiWorldIcons_Begin() {
    LogStubOnce("Patch_UiWorldIcons_Begin");
}

void Patch_UiWorldIcons_End() {
    LogStubOnce("Patch_UiWorldIcons_End");
}

void RpcHook_StateActivate(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("RpcHook_StateActivate");
}

void ShowroomNames_826A9EA8(rex::ppc::Register& r15) {
    (void)r15;
    LogStubOnce("ShowroomNames_826A9EA8");
}

void ShowroomRows_8269381C(rex::ppc::Register& r1) {
    (void)r1;
    LogStubOnce("ShowroomRows_8269381C");
}

void VehicleRecords_8238ACB8(rex::ppc::Register& r6) {
    (void)r6;
    LogStubOnce("VehicleRecords_8238ACB8");
}

void VehicleRecords_8238B40C(rex::ppc::Register& r28) {
    (void)r28;
    LogStubOnce("VehicleRecords_8238B40C");
}

void VehicleRecords_8238C2C0(rex::ppc::Register& r31) {
    (void)r31;
    LogStubOnce("VehicleRecords_8238C2C0");
}

void VehicleRecords_8238C5D4(rex::ppc::Register& r21) {
    (void)r21;
    LogStubOnce("VehicleRecords_8238C5D4");
}

void VehicleRecords_8238C5D8(rex::ppc::Register& r23) {
    (void)r23;
    LogStubOnce("VehicleRecords_8238C5D8");
}

void VehicleRecords_8238C5FC(rex::ppc::Register& r25) {
    (void)r25;
    LogStubOnce("VehicleRecords_8238C5FC");
}

void VehicleRecords_8238C7B4(rex::ppc::Register& r24) {
    (void)r24;
    LogStubOnce("VehicleRecords_8238C7B4");
}

void VehicleRecords_8269A2FC(rex::ppc::Register& r23) {
    (void)r23;
    LogStubOnce("VehicleRecords_8269A2FC");
}

void VehicleRecords_8269B724(rex::ppc::Register& r29) {
    (void)r29;
    LogStubOnce("VehicleRecords_8269B724");
}

void VehicleRecords_8269B810(rex::ppc::Register& r26) {
    (void)r26;
    LogStubOnce("VehicleRecords_8269B810");
}

void VehicleRecords_8269BE7C(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("VehicleRecords_8269BE7C");
}

void VehicleRecords_8269DC84(rex::ppc::Register& r27) {
    (void)r27;
    LogStubOnce("VehicleRecords_8269DC84");
}

void VehicleRecords_8269E438(rex::ppc::Register& r29) {
    (void)r29;
    LogStubOnce("VehicleRecords_8269E438");
}

void VehicleRecords_826A7328(rex::ppc::Register& r22) {
    (void)r22;
    LogStubOnce("VehicleRecords_826A7328");
}

void VehicleRecords_826A96A8(rex::ppc::Register& r27) {
    (void)r27;
    LogStubOnce("VehicleRecords_826A96A8");
}

void VehicleRecords_826A9964(rex::ppc::Register& r7) {
    (void)r7;
    LogStubOnce("VehicleRecords_826A9964");
}

void VehicleRecords_826A9A30(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("VehicleRecords_826A9A30");
}

void VehicleRecords_826A9B0C(rex::ppc::Register& r7) {
    (void)r7;
    LogStubOnce("VehicleRecords_826A9B0C");
}

void VehicleRecords_826A9B84(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("VehicleRecords_826A9B84");
}

void VehicleRecords_826AA490(rex::ppc::Register& r28) {
    (void)r28;
    LogStubOnce("VehicleRecords_826AA490");
}

void VehicleRecords_826AEE00(rex::ppc::Register& r3) {
    (void)r3;
    LogStubOnce("VehicleRecords_826AEE00");
}

void VehicleRecords_826AEF3C(rex::ppc::Register& r29) {
    (void)r29;
    LogStubOnce("VehicleRecords_826AEF3C");
}

void VehicleRecords_826AF6F8(rex::ppc::Register& r5) {
    (void)r5;
    LogStubOnce("VehicleRecords_826AF6F8");
}

void VehicleRecords_826B01B0(rex::ppc::Register& r8) {
    (void)r8;
    LogStubOnce("VehicleRecords_826B01B0");
}

void VehicleRecords_8279ABF0(rex::ppc::Register& r8) {
    (void)r8;
    LogStubOnce("VehicleRecords_8279ABF0");
}