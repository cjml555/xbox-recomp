// MCLA - Game Hook Stubs Implementation
// Generated for M3 milestone
// Stub implementations that allow the app to run on Android

#include "game_hooks.h"
#include <SDL3/SDL.h>

bool Hook_BrowserCreateGameVeto(rex::ppc::Register& r27) {
    (void)r27;
    SDL_Log("Hook_BrowserCreateGameVeto called - stub");
    return false;
}

bool Hook_CarbonPartPop(rex::ppc::Register& r1, rex::ppc::Register& r27, rex::ppc::Register& r31) {
    (void)r1;
    (void)r27;
    (void)r31;
    SDL_Log("Hook_CarbonPartPop called - stub");
    return false;
}

bool Hook_CarbonPartPush(rex::ppc::Register& r1, rex::ppc::Register& r27, rex::ppc::Register& r31) {
    (void)r1;
    (void)r27;
    (void)r31;
    SDL_Log("Hook_CarbonPartPush called - stub");
    return false;
}

bool Hook_CarbonToggle(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_CarbonToggle called - stub");
    return false;
}

bool Hook_ControlCaptureKey(rex::ppc::Register& r11, rex::ppc::Register& r28) {
    (void)r11;
    (void)r28;
    SDL_Log("Hook_ControlCaptureKey called - stub");
    return false;
}

bool Hook_ControlLabel(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r3;
    (void)r4;
    (void)r5;
    SDL_Log("Hook_ControlLabel called - stub");
    return false;
}

bool Hook_ControllerMenuKey(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_ControllerMenuKey called - stub");
    return false;
}

bool Hook_EnablePMSave(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_EnablePMSave called - stub");
    return false;
}

bool Hook_EnablePMTeste(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_EnablePMTeste called - stub");
    return false;
}

bool Hook_FlashCommandLog(rex::ppc::Register& r5) {
    (void)r5;
    SDL_Log("Hook_FlashCommandLog called - stub");
    return false;
}

bool Hook_LanAccept(rex::ppc::Register& r30, rex::ppc::Register& r28) {
    (void)r30;
    (void)r28;
    SDL_Log("Hook_LanAccept called - stub");
    return false;
}

bool Hook_ListViewPopulate(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ListViewPopulate called - stub");
    return false;
}

bool Hook_MenuCameraPick(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_MenuCameraPick called - stub");
    return false;
}

bool Hook_MenuKeyDispatch(rex::ppc::Register& r31, rex::ppc::Register& r28, rex::ppc::Register& r27) {
    (void)r31;
    (void)r28;
    (void)r27;
    SDL_Log("Hook_MenuKeyDispatch called - stub");
    return false;
}

bool Hook_PMLodTrafficClick(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    SDL_Log("Hook_PMLodTrafficClick called - stub");
    return false;
}

bool Hook_PopulateRedirect(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_PopulateRedirect called - stub");
    return false;
}

bool Hook_PumpWake(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_PumpWake called - stub");
    return false;
}

bool Hook_PumpWakeRingWait(rex::ppc::Register& r31, rex::ppc::Register& r30, rex::ppc::Register& r29, rex::ppc::Register& r26, rex::ppc::Register& r3) {
    (void)r31;
    (void)r30;
    (void)r29;
    (void)r26;
    (void)r3;
    SDL_Log("Hook_PumpWakeRingWait called - stub");
    return false;
}

bool Hook_RexGlueCancel(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("Hook_RexGlueCancel called - stub");
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
    SDL_Log("Hook_RmrDownloadFile called - stub");
    return false;
}

bool Hook_RmrGetTableSize(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    SDL_Log("Hook_RmrGetTableSize called - stub");
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
    SDL_Log("Hook_RmrOverwriteFile called - stub");
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
    SDL_Log("Hook_RmrOverwriteUserData called - stub");
    return false;
}

bool Hook_RmrRateRecord(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6, rex::ppc::Register& r7, rex::ppc::Register& r8) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    (void)r7;
    (void)r8;
    SDL_Log("Hook_RmrRateRecord called - stub");
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
    SDL_Log("Hook_RmrReadByGamer called - stub");
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
    SDL_Log("Hook_RmrReadByRank called - stub");
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
    SDL_Log("Hook_RmrReadRandom called - stub");
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
    SDL_Log("Hook_RmrUploadFile called - stub");
    return false;
}

bool Hook_StringTableLanguage(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_StringTableLanguage called - stub");
    return false;
}

bool MCLA_CarShaderBindDone_823794C8(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_CarShaderBindDone_823794C8 called - stub");
    return false;
}

bool MCLA_CarShaderBindDone_82379548(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_CarShaderBindDone_82379548 called - stub");
    return false;
}

bool MCLA_CustomMusic_StartSong(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_CustomMusic_StartSong called - stub");
    return false;
}

bool MCLA_MapCursorMagnetGuard() {
    SDL_Log("MCLA_MapCursorMagnetGuard called - stub");
    return false;
}

bool MCLA_MapCursorSweepNullGuard(rex::ppc::Register& r31, rex::ppc::Register& r30, rex::ppc::Register& r26) {
    (void)r31;
    (void)r30;
    (void)r26;
    SDL_Log("MCLA_MapCursorSweepNullGuard called - stub");
    return false;
}

bool MCLA_MenuListNullTableGuard(rex::ppc::Register& r30) {
    (void)r30;
    SDL_Log("MCLA_MenuListNullTableGuard called - stub");
    return false;
}

bool MCLA_PauseListVtableGuard(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_PauseListVtableGuard called - stub");
    return false;
}

bool MCLA_ShaderGroupLookup_82193AF8(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLA_ShaderGroupLookup_82193AF8 called - stub");
    return false;
}

bool MCLA_TrafficBoundRelease_8259AA40(rex::ppc::Register& r30) {
    (void)r30;
    SDL_Log("MCLA_TrafficBoundRelease_8259AA40 called - stub");
    return false;
}

bool MCLA_UI_SkipMissingLights(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLA_UI_SkipMissingLights called - stub");
    return false;
}

bool MCLAUseRealDelta() {
    SDL_Log("MCLAUseRealDelta called - stub");
    return false;
}

bool Patch_60FPS_Byte(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Patch_60FPS_Byte called - stub");
    return false;
}

bool Patch_AspectRatio_82214BB8(rex::ppc::Register& f10) {
    (void)f10;
    SDL_Log("Patch_AspectRatio_82214BB8 called - stub");
    return false;
}

bool Patch_AspectRatio_82233EB4(rex::ppc::Register& f0, rex::ppc::Register& f30) {
    (void)f0;
    (void)f30;
    SDL_Log("Patch_AspectRatio_82233EB4 called - stub");
    return false;
}

bool Patch_AspectRatio_8223E5E0(rex::ppc::Register& f13) {
    (void)f13;
    SDL_Log("Patch_AspectRatio_8223E5E0 called - stub");
    return false;
}

bool Patch_AspectRatio_822C0918(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("Patch_AspectRatio_822C0918 called - stub");
    return false;
}

bool Patch_AspectRatio_822E5E68(rex::ppc::Register& f12, rex::ppc::Register& f30) {
    (void)f12;
    (void)f30;
    SDL_Log("Patch_AspectRatio_822E5E68 called - stub");
    return false;
}

bool Patch_BypassContentCheck(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r3;
    (void)r4;
    (void)r5;
    SDL_Log("Patch_BypassContentCheck called - stub");
    return false;
}

bool Patch_DebugCamGate() {
    SDL_Log("Patch_DebugCamGate called - stub");
    return false;
}

bool Patch_DisableImposterShadows(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Patch_DisableImposterShadows called - stub");
    return false;
}

bool Patch_DisableMotionBlur(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_DisableMotionBlur called - stub");
    return false;
}

bool Patch_DisableMSAA(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Patch_DisableMSAA called - stub");
    return false;
}

bool Patch_DisableRubberBanding() {
    SDL_Log("Patch_DisableRubberBanding called - stub");
    return false;
}

bool Patch_EdramLimit(rex::ppc::Register& r3, rex::ppc::Register& r30, rex::ppc::Register& r11) {
    (void)r3;
    (void)r30;
    (void)r11;
    SDL_Log("Patch_EdramLimit called - stub");
    return false;
}

bool Patch_FenceSpinThrottle() {
    SDL_Log("Patch_FenceSpinThrottle called - stub");
    return false;
}

bool Patch_ImpostorShadowGuard(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_ImpostorShadowGuard called - stub");
    return false;
}

bool Patch_MapVisibleAspect_82620700(rex::ppc::Register& f27) {
    (void)f27;
    SDL_Log("Patch_MapVisibleAspect_82620700 called - stub");
    return false;
}

bool Patch_PhysicsCollision() {
    SDL_Log("Patch_PhysicsCollision called - stub");
    return false;
}

bool Patch_PlatformPush(rex::ppc::Register& r5) {
    (void)r5;
    SDL_Log("Patch_PlatformPush called - stub");
    return false;
}

bool Patch_PropOwnerPoseGuard(rex::ppc::Register& r31, rex::ppc::Register& r4) {
    (void)r31;
    (void)r4;
    SDL_Log("Patch_PropOwnerPoseGuard called - stub");
    return false;
}

bool Patch_PropOwnerSyncGuard(rex::ppc::Register& r31, rex::ppc::Register& r4) {
    (void)r31;
    (void)r4;
    SDL_Log("Patch_PropOwnerSyncGuard called - stub");
    return false;
}

bool Patch_RearViewFrameSkip() {
    SDL_Log("Patch_RearViewFrameSkip called - stub");
    return false;
}

bool Patch_SpeedUnits(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Patch_SpeedUnits called - stub");
    return false;
}

bool Patch_WheelFitBypass() {
    SDL_Log("Patch_WheelFitBypass called - stub");
    return false;
}

bool SkipIntro() {
    SDL_Log("SkipIntro called - stub");
    return false;
}

void Hook_BrowserJoinSession(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_BrowserJoinSession called - stub");
}

void Hook_BrowserLanResult(rex::ppc::Register& r1, rex::ppc::Register& r27) {
    (void)r1;
    (void)r27;
    SDL_Log("Hook_BrowserLanResult called - stub");
}

void Hook_BrowserLiveResult(rex::ppc::Register& r30, rex::ppc::Register& r3) {
    (void)r30;
    (void)r3;
    SDL_Log("Hook_BrowserLiveResult called - stub");
}

void Hook_CacheVinylPaint(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    SDL_Log("Hook_CacheVinylPaint called - stub");
}

void Hook_CameraMatrixPublish(rex::ppc::Register& r30) {
    (void)r30;
    SDL_Log("Hook_CameraMatrixPublish called - stub");
}

void Hook_CameraPostUpdate(rex::ppc::Register& r20) {
    (void)r20;
    SDL_Log("Hook_CameraPostUpdate called - stub");
}

void Hook_CameraPrePublish(rex::ppc::Register& r20) {
    (void)r20;
    SDL_Log("Hook_CameraPrePublish called - stub");
}

void Hook_CameraStateUpdate(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("Hook_CameraStateUpdate called - stub");
}

void Hook_CameraSystem(rex::ppc::Register& r20, rex::ppc::Register& r24) {
    (void)r20;
    (void)r24;
    SDL_Log("Hook_CameraSystem called - stub");
}

void Hook_CaptureDistrict(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_CaptureDistrict called - stub");
}

void Hook_CapturePMContinue(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_CapturePMContinue called - stub");
}

void Hook_CarbonGarageProbe(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_CarbonGarageProbe called - stub");
}

void Hook_CarbonGarageSelect(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    SDL_Log("Hook_CarbonGarageSelect called - stub");
}

void Hook_CarbonUiTick(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_CarbonUiTick called - stub");
}

void Hook_ChallengeClockGate(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ChallengeClockGate called - stub");
}

void Hook_ChallengeHostPublishGate(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_ChallengeHostPublishGate called - stub");
}

void Hook_ChallengeIsGroupHost(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ChallengeIsGroupHost called - stub");
}

void Hook_ChallengeProp3Gate(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ChallengeProp3Gate called - stub");
}

void Hook_ChallengeProposeRoute(rex::ppc::Register& r3, rex::ppc::Register& r29) {
    (void)r3;
    (void)r29;
    SDL_Log("Hook_ChallengeProposeRoute called - stub");
}

void Hook_ChallengeResolveChallenger(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ChallengeResolveChallenger called - stub");
}

void Hook_ChallengeTimeGate(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_ChallengeTimeGate called - stub");
}

void Hook_ClockAmOwner(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ClockAmOwner called - stub");
}

void Hook_ClockOwnerRecord(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("Hook_ClockOwnerRecord called - stub");
}

void Hook_ClockResolveOwner(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ClockResolveOwner called - stub");
}

void Hook_ClockSessionReady(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ClockSessionReady called - stub");
}

void Hook_ClockStarting(rex::ppc::Register& r5, rex::ppc::Register& r7) {
    (void)r5;
    (void)r7;
    SDL_Log("Hook_ClockStarting called - stub");
}

void Hook_ControlMapRebuilt(rex::ppc::Register& r30) {
    (void)r30;
    SDL_Log("Hook_ControlMapRebuilt called - stub");
}

void Hook_DisplayGammaType(rex::ppc::Register& r1) {
    (void)r1;
    SDL_Log("Hook_DisplayGammaType called - stub");
}

void Hook_DumpJoinBlob(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("Hook_DumpJoinBlob called - stub");
}

void Hook_InviteReceiveProbe(rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r4;
    (void)r5;
    SDL_Log("Hook_InviteReceiveProbe called - stub");
}

void Hook_InviteSendProbe(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_InviteSendProbe called - stub");
}

void Hook_KeyboardFeed() {
    SDL_Log("Hook_KeyboardFeed called - stub");
}

void Hook_LanFillElement(rex::ppc::Register& r27, rex::ppc::Register& r28, rex::ppc::Register& r30) {
    (void)r27;
    (void)r28;
    (void)r30;
    SDL_Log("Hook_LanFillElement called - stub");
}

void Hook_LeaderboardWrite(rex::ppc::Register& r25, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r25;
    (void)r5;
    (void)r6;
    SDL_Log("Hook_LeaderboardWrite called - stub");
}

void Hook_LzxDecompressPost(rex::ppc::Register& r1, rex::ppc::Register& r3) {
    (void)r1;
    (void)r3;
    SDL_Log("Hook_LzxDecompressPost called - stub");
}

void Hook_LzxDecompressPre(rex::ppc::Register& r1) {
    (void)r1;
    SDL_Log("Hook_LzxDecompressPre called - stub");
}

void Hook_MapInputTick(rex::ppc::Register& r31, rex::ppc::Register& f1) {
    (void)r31;
    (void)f1;
    SDL_Log("Hook_MapInputTick called - stub");
}

void Hook_MenuCameraFinal(rex::ppc::Register& r1, rex::ppc::Register& r24) {
    (void)r1;
    (void)r24;
    SDL_Log("Hook_MenuCameraFinal called - stub");
}

void Hook_NetBigSessionDefault(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_NetBigSessionDefault called - stub");
}

void Hook_NetBigSessionFlag(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_NetBigSessionFlag called - stub");
}

void Hook_NetLinkWhileDown(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_NetLinkWhileDown called - stub");
}

void Hook_NetLinkWhileUp(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_NetLinkWhileUp called - stub");
}

void Hook_PadBitsFinalize(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("Hook_PadBitsFinalize called - stub");
}

void Hook_PhotoModeCapture(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_PhotoModeCapture called - stub");
}

void Hook_PlatformVarInit(rex::ppc::Register& r27) {
    (void)r27;
    SDL_Log("Hook_PlatformVarInit called - stub");
}

void Hook_PromptDropped() {
    SDL_Log("Hook_PromptDropped called - stub");
}

void Hook_PromptEntryGate(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_PromptEntryGate called - stub");
}

void Hook_PromptFlowGate(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_PromptFlowGate called - stub");
}

void Hook_PromptRaised() {
    SDL_Log("Hook_PromptRaised called - stub");
}

void Hook_PromptStateIdle(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_PromptStateIdle called - stub");
}

void Hook_PromptTableGate(rex::ppc::Register& r3, rex::ppc::Register& r25) {
    (void)r3;
    (void)r25;
    SDL_Log("Hook_PromptTableGate called - stub");
}

void Hook_PromptTimeGate(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_PromptTimeGate called - stub");
}

void Hook_PropOwnerCreated(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    SDL_Log("Hook_PropOwnerCreated called - stub");
}

void Hook_PropOwnerFreed(rex::ppc::Register& r31, rex::ppc::Register& r1) {
    (void)r31;
    (void)r1;
    SDL_Log("Hook_PropOwnerFreed called - stub");
}

void Hook_PumpWakeMixerDone() {
    SDL_Log("Hook_PumpWakeMixerDone called - stub");
}

void Hook_PumpWakeSlotFreed() {
    SDL_Log("Hook_PumpWakeSlotFreed called - stub");
}

void Hook_RmrEnter(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_RmrEnter called - stub");
}

void Hook_RmrPump(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_RmrPump called - stub");
}

void Hook_RoadReflCarsEnter(rex::ppc::Register& r30, rex::ppc::Register& r28, rex::ppc::Register& r13) {
    (void)r30;
    (void)r28;
    (void)r13;
    SDL_Log("Hook_RoadReflCarsEnter called - stub");
}

void Hook_RoadReflCarsGate(rex::ppc::Register& r10) {
    (void)r10;
    SDL_Log("Hook_RoadReflCarsGate called - stub");
}

void Hook_RoadReflCarsPlane(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("Hook_RoadReflCarsPlane called - stub");
}

void Hook_ScriptButtonRead(rex::ppc::Register& r3, rex::ppc::Register& r30, rex::ppc::Register& r7, rex::ppc::Register& r11) {
    (void)r3;
    (void)r30;
    (void)r7;
    (void)r11;
    SDL_Log("Hook_ScriptButtonRead called - stub");
}

void Hook_ScriptButtonRefused() {
    SDL_Log("Hook_ScriptButtonRefused called - stub");
}

void Hook_ScriptHighbeamsAnswer(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ScriptHighbeamsAnswer called - stub");
}

void Hook_ScriptHighbeamsAsk(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_ScriptHighbeamsAsk called - stub");
}

void Hook_ScriptHighbeamsRefused() {
    SDL_Log("Hook_ScriptHighbeamsRefused called - stub");
}

void Hook_ScriptPadRead(rex::ppc::Register& r8, rex::ppc::Register& r5, rex::ppc::Register& r3, rex::ppc::Register& r6) {
    (void)r8;
    (void)r5;
    (void)r3;
    (void)r6;
    SDL_Log("Hook_ScriptPadRead called - stub");
}

void Hook_ScriptUiPressedAnswer(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_ScriptUiPressedAnswer called - stub");
}

void Hook_ScriptUiPressedAsk(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Hook_ScriptUiPressedAsk called - stub");
}

void Hook_SessionCreateObject(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_SessionCreateObject called - stub");
}

void Hook_SquashInjectMatrix(rex::ppc::Register& r4, rex::ppc::Register& r6) {
    (void)r4;
    (void)r6;
    SDL_Log("Hook_SquashInjectMatrix called - stub");
}

void Hook_SquashSelectVehicle(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_SquashSelectVehicle called - stub");
}

void Hook_SquashWorldMatrix(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("Hook_SquashWorldMatrix called - stub");
}

void Hook_SwfContextEnter(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_SwfContextEnter called - stub");
}

void Hook_TimeOfDay(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_TimeOfDay called - stub");
}

void Hook_TireOffsetWheels(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("Hook_TireOffsetWheels called - stub");
}

void Hook_TournamentPump(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_TournamentPump called - stub");
}

void Hook_TriggerLineSweep(rex::ppc::Register& r1, rex::ppc::Register& r31, rex::ppc::Register& r27) {
    (void)r1;
    (void)r31;
    (void)r27;
    SDL_Log("Hook_TriggerLineSweep called - stub");
}

void Hook_TriggersHud(rex::ppc::Register& r30) {
    (void)r30;
    SDL_Log("Hook_TriggersHud called - stub");
}

void Hook_UiCamCtrl(rex::ppc::Register& r31, rex::ppc::Register& f0) {
    (void)r31;
    (void)f0;
    SDL_Log("Hook_UiCamCtrl called - stub");
}

void Hook_UiCamDone() {
    SDL_Log("Hook_UiCamDone called - stub");
}

void Hook_UiColor_Edit(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_UiColor_Edit called - stub");
}

void Hook_UiColor_Fill(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_UiColor_Fill called - stub");
}

void Hook_UiColor_Line(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_UiColor_Line called - stub");
}

void Hook_UiColor_TextInit(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_UiColor_TextInit called - stub");
}

void Hook_UiColor_TextRun(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("Hook_UiColor_TextRun called - stub");
}

void Hook_UiCountdownDt(rex::ppc::Register& r3, rex::ppc::Register& f1) {
    (void)r3;
    (void)f1;
    SDL_Log("Hook_UiCountdownDt called - stub");
}

void Hook_UiSceneEnter(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Hook_UiSceneEnter called - stub");
}

void Hook_UiSceneFloat(rex::ppc::Register& r1, rex::ppc::Register& r30, rex::ppc::Register& r31) {
    (void)r1;
    (void)r30;
    (void)r31;
    SDL_Log("Hook_UiSceneFloat called - stub");
}

void Hook_UiSceneLeave() {
    SDL_Log("Hook_UiSceneLeave called - stub");
}

void Hook_UiSceneVec(rex::ppc::Register& r1, rex::ppc::Register& r30, rex::ppc::Register& r31) {
    (void)r1;
    (void)r30;
    (void)r31;
    SDL_Log("Hook_UiSceneVec called - stub");
}

void Hook_UiSmoothDraw(rex::ppc::Register& r30, rex::ppc::Register& r31) {
    (void)r30;
    (void)r31;
    SDL_Log("Hook_UiSmoothDraw called - stub");
}

void Hook_VibrationSet(rex::ppc::Register& r3, rex::ppc::Register& r5) {
    (void)r3;
    (void)r5;
    SDL_Log("Hook_VibrationSet called - stub");
}

void Hook_WheelHudGear(rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r4;
    (void)r5;
    SDL_Log("Hook_WheelHudGear called - stub");
}

void Hook_WheelSlip(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("Hook_WheelSlip called - stub");
}

void MCLAAmbientDensityTuning(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLAAmbientDensityTuning called - stub");
}

void MCLA_Audio_BankError(rex::ppc::Register& r4, rex::ppc::Register& r31) {
    (void)r4;
    (void)r31;
    SDL_Log("MCLA_Audio_BankError called - stub");
}

void MCLA_Audio_BankOpened(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    SDL_Log("MCLA_Audio_BankOpened called - stub");
}

void MCLAAudioClockAccum(rex::ppc::Register& r9, rex::ppc::Register& r29, rex::ppc::Register& f0) {
    (void)r9;
    (void)r29;
    (void)f0;
    SDL_Log("MCLAAudioClockAccum called - stub");
}

void MCLABikeGyroPositionFilter(rex::ppc::Register& f13) {
    (void)f13;
    SDL_Log("MCLABikeGyroPositionFilter called - stub");
}

void MCLABikeGyroVelocityFilter(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("MCLABikeGyroVelocityFilter called - stub");
}

void MCLACameraBoomFilters(rex::ppc::Register& f13, rex::ppc::Register& f9, rex::ppc::Register& f1) {
    (void)f13;
    (void)f9;
    (void)f1;
    SDL_Log("MCLACameraBoomFilters called - stub");
}

void MCLACameraLookAtSmoothing(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("MCLACameraLookAtSmoothing called - stub");
}

void MCLACameraPosSmoothing(rex::ppc::Register& f13) {
    (void)f13;
    SDL_Log("MCLACameraPosSmoothing called - stub");
}

void MCLACameraShakeFilter(rex::ppc::Register& f31, rex::ppc::Register& f30, rex::ppc::Register& f29, rex::ppc::Register& f0, rex::ppc::Register& r1) {
    (void)f31;
    (void)f30;
    (void)f29;
    (void)f0;
    (void)r1;
    SDL_Log("MCLACameraShakeFilter called - stub");
}

void MCLACameraTransitionComplete(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLACameraTransitionComplete called - stub");
}

void MCLACameraTransitionSmoothing(rex::ppc::Register& r27, rex::ppc::Register& r26, rex::ppc::Register& r4, rex::ppc::Register& r1) {
    (void)r27;
    (void)r26;
    (void)r4;
    (void)r1;
    SDL_Log("MCLACameraTransitionSmoothing called - stub");
}

void MCLA_CamEventClip(rex::ppc::Register& r30, rex::ppc::Register& r28, rex::ppc::Register& r27) {
    (void)r30;
    (void)r28;
    (void)r27;
    SDL_Log("MCLA_CamEventClip called - stub");
}

void MCLA_CamEventEnter(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("MCLA_CamEventEnter called - stub");
}

void MCLA_CamEventNoPack() {
    SDL_Log("MCLA_CamEventNoPack called - stub");
}

void MCLA_CamEventPool(rex::ppc::Register& r29) {
    (void)r29;
    SDL_Log("MCLA_CamEventPool called - stub");
}

void MCLA_CamEventQueued(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("MCLA_CamEventQueued called - stub");
}

void MCLAChassisDepthSmoothing(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("MCLAChassisDepthSmoothing called - stub");
}

void MCLAChassisGroundPushbackDamper(rex::ppc::Register& f0, rex::ppc::Register& f3) {
    (void)f0;
    (void)f3;
    SDL_Log("MCLAChassisGroundPushbackDamper called - stub");
}

void MCLAChassisPushbackAccel(rex::ppc::Register& f1) {
    (void)f1;
    SDL_Log("MCLAChassisPushbackAccel called - stub");
}

void MCLA_CineActorBound(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("MCLA_CineActorBound called - stub");
}

void MCLA_CineActorClip(rex::ppc::Register& r20) {
    (void)r20;
    SDL_Log("MCLA_CineActorClip called - stub");
}

void MCLA_CineActorEntity(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_CineActorEntity called - stub");
}

void MCLA_CineActorName(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("MCLA_CineActorName called - stub");
}

void MCLA_CineActorStart(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("MCLA_CineActorStart called - stub");
}

void MCLA_CineCamInstall(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r3;
    (void)r4;
    (void)r5;
    SDL_Log("MCLA_CineCamInstall called - stub");
}

void MCLA_CineCamUpdate(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLA_CineCamUpdate called - stub");
}

void MCLA_CineCharActor(rex::ppc::Register& r18) {
    (void)r18;
    SDL_Log("MCLA_CineCharActor called - stub");
}

void MCLA_CineCharAnim(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    SDL_Log("MCLA_CineCharAnim called - stub");
}

void MCLA_CineCharLoaded(rex::ppc::Register& r8) {
    (void)r8;
    SDL_Log("MCLA_CineCharLoaded called - stub");
}

void MCLA_CineCharLoad(rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r4;
    (void)r5;
    SDL_Log("MCLA_CineCharLoad called - stub");
}

void MCLA_CineCharQueued(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("MCLA_CineCharQueued called - stub");
}

void MCLA_CineEventFired(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("MCLA_CineEventFired called - stub");
}

void MCLA_CineEventTime(rex::ppc::Register& f1, rex::ppc::Register& r31) {
    (void)f1;
    (void)r31;
    SDL_Log("MCLA_CineEventTime called - stub");
}

void MCLA_CineEventWalk(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_CineEventWalk called - stub");
}

void MCLA_CustomMusic_Announce(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("MCLA_CustomMusic_Announce called - stub");
}

void MCLA_CustomMusic_CueGain(rex::ppc::Register& r4, rex::ppc::Register& r6) {
    (void)r4;
    (void)r6;
    SDL_Log("MCLA_CustomMusic_CueGain called - stub");
}

void MCLA_CustomMusic_Install(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_CustomMusic_Install called - stub");
}

void MCLA_CustomMusic_Pause(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLA_CustomMusic_Pause called - stub");
}

void MCLA_CustomMusic_SelectSong(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLA_CustomMusic_SelectSong called - stub");
}

void MCLA_CustomMusic_Stop(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLA_CustomMusic_Stop called - stub");
}

void MCLA_DriverAnimDict(rex::ppc::Register& r28, rex::ppc::Register& r29) {
    (void)r28;
    (void)r29;
    SDL_Log("MCLA_DriverAnimDict called - stub");
}

void MCLA_DriverAnimName(rex::ppc::Register& r1) {
    (void)r1;
    SDL_Log("MCLA_DriverAnimName called - stub");
}

void MCLA_DriverAnimPack(rex::ppc::Register& r1) {
    (void)r1;
    SDL_Log("MCLA_DriverAnimPack called - stub");
}

void MCLA_FileOpenResult(rex::ppc::Register& r1, rex::ppc::Register& r3) {
    (void)r1;
    (void)r3;
    SDL_Log("MCLA_FileOpenResult called - stub");
}

void MCLAFixedStepPath(rex::ppc::Register& r3, rex::ppc::Register& f11) {
    (void)r3;
    (void)f11;
    SDL_Log("MCLAFixedStepPath called - stub");
}

void MCLAFrameDelta(rex::ppc::Register& r8) {
    (void)r8;
    SDL_Log("MCLAFrameDelta called - stub");
}

void MCLAFrameStart() {
    SDL_Log("MCLAFrameStart called - stub");
}

void MCLA_GameCamActive(rex::ppc::Register& r11) {
    (void)r11;
    SDL_Log("MCLA_GameCamActive called - stub");
}

void MCLA_GameCamInGame(rex::ppc::Register& r19) {
    (void)r19;
    SDL_Log("MCLA_GameCamInGame called - stub");
}

void MCLAGameClockCarry(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("MCLAGameClockCarry called - stub");
}

void MCLA_GuestInterruptProbe(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    SDL_Log("MCLA_GuestInterruptProbe called - stub");
}

void MCLA_InflateStep(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("MCLA_InflateStep called - stub");
}

void MCLA_RageFatal(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    SDL_Log("MCLA_RageFatal called - stub");
}

void MCLA_ResourceFixupError(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5) {
    (void)r3;
    (void)r4;
    (void)r5;
    SDL_Log("MCLA_ResourceFixupError called - stub");
}

void MCLA_ResourceSegments(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("MCLA_ResourceSegments called - stub");
}

void MCLA_SkipIntroRenderPassMask(rex::ppc::Register& r4) {
    (void)r4;
    SDL_Log("MCLA_SkipIntroRenderPassMask called - stub");
}

void MCLA_StreamBufferProbe(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("MCLA_StreamBufferProbe called - stub");
}

void MCLA_StreamOpenResult(rex::ppc::Register& r1, rex::ppc::Register& r3) {
    (void)r1;
    (void)r3;
    SDL_Log("MCLA_StreamOpenResult called - stub");
}

void MCLATimeAccumGame(rex::ppc::Register& r3, rex::ppc::Register& f0) {
    (void)r3;
    (void)f0;
    SDL_Log("MCLATimeAccumGame called - stub");
}

void MCLATimeAccumTotal(rex::ppc::Register& r3, rex::ppc::Register& f0) {
    (void)r3;
    (void)f0;
    SDL_Log("MCLATimeAccumTotal called - stub");
}

void MCLATimeScaleOverride(rex::ppc::Register& f13) {
    (void)f13;
    SDL_Log("MCLATimeScaleOverride called - stub");
}

void MCLATireSlipSmoothing(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("MCLATireSlipSmoothing called - stub");
}

void MCLA_TrafficChassisBound_8232D048(rex::ppc::Register& r9, rex::ppc::Register& r11) {
    (void)r9;
    (void)r11;
    SDL_Log("MCLA_TrafficChassisBound_8232D048 called - stub");
}

void MCLA_TrafficChassisBound_8232D900(rex::ppc::Register& r3, rex::ppc::Register& r11) {
    (void)r3;
    (void)r11;
    SDL_Log("MCLA_TrafficChassisBound_8232D900 called - stub");
}

void MCLA_TrafficChassisBound_8232E274(rex::ppc::Register& r3, rex::ppc::Register& r31) {
    (void)r3;
    (void)r31;
    SDL_Log("MCLA_TrafficChassisBound_8232E274 called - stub");
}

void MCLA_TuneFieldProbe(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r5, rex::ppc::Register& r6) {
    (void)r3;
    (void)r4;
    (void)r5;
    (void)r6;
    SDL_Log("MCLA_TuneFieldProbe called - stub");
}

void Patch_AmbientCascadeClient(rex::ppc::Register& r11, rex::ppc::Register& r30, rex::ppc::Register& r31) {
    (void)r11;
    (void)r30;
    (void)r31;
    SDL_Log("Patch_AmbientCascadeClient called - stub");
}

void Patch_AmbientCascadeDraw(rex::ppc::Register& r3, rex::ppc::Register& r4, rex::ppc::Register& r13) {
    (void)r3;
    (void)r4;
    (void)r13;
    SDL_Log("Patch_AmbientCascadeDraw called - stub");
}

void Patch_ArchiveList(rex::ppc::Register& r1) {
    (void)r1;
    SDL_Log("Patch_ArchiveList called - stub");
}

void Patch_BypassVehicleDLC(rex::ppc::Register& r30) {
    (void)r30;
    SDL_Log("Patch_BypassVehicleDLC called - stub");
}

void Patch_CutsceneFovAspect_822C04E8(rex::ppc::Register& f1) {
    (void)f1;
    SDL_Log("Patch_CutsceneFovAspect_822C04E8 called - stub");
}

void Patch_DebugCam(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_DebugCam called - stub");
}

void Patch_DeltaTime(rex::ppc::Register& r24) {
    (void)r24;
    SDL_Log("Patch_DeltaTime called - stub");
}

void Patch_DeltaTimePre() {
    SDL_Log("Patch_DeltaTimePre called - stub");
}

void Patch_DevOptionRegistered(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_DevOptionRegistered called - stub");
}

void Patch_DofComposite(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_DofComposite called - stub");
}

void Patch_FOVScale(rex::ppc::Register& f1, rex::ppc::Register& r24) {
    (void)f1;
    (void)r24;
    SDL_Log("Patch_FOVScale called - stub");
}

void Patch_MapVisibleHalfFov_82620708(rex::ppc::Register& f31) {
    (void)f31;
    SDL_Log("Patch_MapVisibleHalfFov_82620708 called - stub");
}

void Patch_RearViewCarPhase(rex::ppc::Register& r3, rex::ppc::Register& r4) {
    (void)r3;
    (void)r4;
    SDL_Log("Patch_RearViewCarPhase called - stub");
}

void Patch_RearViewCullBits(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_RearViewCullBits called - stub");
}

void Patch_RearViewDiag() {
    SDL_Log("Patch_RearViewDiag called - stub");
}

void Patch_RearViewDrawAlpha(rex::ppc::Register& f13) {
    (void)f13;
    SDL_Log("Patch_RearViewDrawAlpha called - stub");
}

void Patch_RearViewUpdateActive(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_RearViewUpdateActive called - stub");
}

void Patch_RearViewUpdateVisible(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_RearViewUpdateVisible called - stub");
}

void Patch_RideHeightFit(rex::ppc::Register& f30) {
    (void)f30;
    SDL_Log("Patch_RideHeightFit called - stub");
}

void Patch_ScaleTrafficLOD(rex::ppc::Register& f0) {
    (void)f0;
    SDL_Log("Patch_ScaleTrafficLOD called - stub");
}

void Patch_SingleTile(rex::ppc::Register& r7, rex::ppc::Register& r8, rex::ppc::Register& r17, rex::ppc::Register& r25, rex::ppc::Register& r28) {
    (void)r7;
    (void)r8;
    (void)r17;
    (void)r25;
    (void)r28;
    SDL_Log("Patch_SingleTile called - stub");
}

void Patch_SpeedLimitUnits(rex::ppc::Register& r8) {
    (void)r8;
    SDL_Log("Patch_SpeedLimitUnits called - stub");
}

void Patch_TrafficCascadeGate(rex::ppc::Register& r11, rex::ppc::Register& r13, rex::ppc::Register& r28, rex::ppc::Register& r30) {
    (void)r11;
    (void)r13;
    (void)r28;
    (void)r30;
    SDL_Log("Patch_TrafficCascadeGate called - stub");
}

void Patch_UiMarkerX_82289808(rex::ppc::Register& f7) {
    (void)f7;
    SDL_Log("Patch_UiMarkerX_82289808 called - stub");
}

void Patch_UiMarkerY_82289810(rex::ppc::Register& f8) {
    (void)f8;
    SDL_Log("Patch_UiMarkerY_82289810 called - stub");
}

void Patch_UiPiece_Begin(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("Patch_UiPiece_Begin called - stub");
}

void Patch_UiPiece_End(rex::ppc::Register& r30) {
    (void)r30;
    SDL_Log("Patch_UiPiece_End called - stub");
}

void Patch_UiWorldIcons_Begin() {
    SDL_Log("Patch_UiWorldIcons_Begin called - stub");
}

void Patch_UiWorldIcons_End() {
    SDL_Log("Patch_UiWorldIcons_End called - stub");
}

void RpcHook_StateActivate(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("RpcHook_StateActivate called - stub");
}

void ShowroomNames_826A9EA8(rex::ppc::Register& r15) {
    (void)r15;
    SDL_Log("ShowroomNames_826A9EA8 called - stub");
}

void ShowroomRows_8269381C(rex::ppc::Register& r1) {
    (void)r1;
    SDL_Log("ShowroomRows_8269381C called - stub");
}

void VehicleRecords_8238ACB8(rex::ppc::Register& r6) {
    (void)r6;
    SDL_Log("VehicleRecords_8238ACB8 called - stub");
}

void VehicleRecords_8238B40C(rex::ppc::Register& r28) {
    (void)r28;
    SDL_Log("VehicleRecords_8238B40C called - stub");
}

void VehicleRecords_8238C2C0(rex::ppc::Register& r31) {
    (void)r31;
    SDL_Log("VehicleRecords_8238C2C0 called - stub");
}

void VehicleRecords_8238C5D4(rex::ppc::Register& r21) {
    (void)r21;
    SDL_Log("VehicleRecords_8238C5D4 called - stub");
}

void VehicleRecords_8238C5D8(rex::ppc::Register& r23) {
    (void)r23;
    SDL_Log("VehicleRecords_8238C5D8 called - stub");
}

void VehicleRecords_8238C5FC(rex::ppc::Register& r25) {
    (void)r25;
    SDL_Log("VehicleRecords_8238C5FC called - stub");
}

void VehicleRecords_8238C7B4(rex::ppc::Register& r24) {
    (void)r24;
    SDL_Log("VehicleRecords_8238C7B4 called - stub");
}

void VehicleRecords_8269A2FC(rex::ppc::Register& r23) {
    (void)r23;
    SDL_Log("VehicleRecords_8269A2FC called - stub");
}

void VehicleRecords_8269B724(rex::ppc::Register& r29) {
    (void)r29;
    SDL_Log("VehicleRecords_8269B724 called - stub");
}

void VehicleRecords_8269B810(rex::ppc::Register& r26) {
    (void)r26;
    SDL_Log("VehicleRecords_8269B810 called - stub");
}

void VehicleRecords_8269BE7C(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("VehicleRecords_8269BE7C called - stub");
}

void VehicleRecords_8269DC84(rex::ppc::Register& r27) {
    (void)r27;
    SDL_Log("VehicleRecords_8269DC84 called - stub");
}

void VehicleRecords_8269E438(rex::ppc::Register& r29) {
    (void)r29;
    SDL_Log("VehicleRecords_8269E438 called - stub");
}

void VehicleRecords_826A7328(rex::ppc::Register& r22) {
    (void)r22;
    SDL_Log("VehicleRecords_826A7328 called - stub");
}

void VehicleRecords_826A96A8(rex::ppc::Register& r27) {
    (void)r27;
    SDL_Log("VehicleRecords_826A96A8 called - stub");
}

void VehicleRecords_826A9964(rex::ppc::Register& r7) {
    (void)r7;
    SDL_Log("VehicleRecords_826A9964 called - stub");
}

void VehicleRecords_826A9A30(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("VehicleRecords_826A9A30 called - stub");
}

void VehicleRecords_826A9B0C(rex::ppc::Register& r7) {
    (void)r7;
    SDL_Log("VehicleRecords_826A9B0C called - stub");
}

void VehicleRecords_826A9B84(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("VehicleRecords_826A9B84 called - stub");
}

void VehicleRecords_826AA490(rex::ppc::Register& r28) {
    (void)r28;
    SDL_Log("VehicleRecords_826AA490 called - stub");
}

void VehicleRecords_826AEE00(rex::ppc::Register& r3) {
    (void)r3;
    SDL_Log("VehicleRecords_826AEE00 called - stub");
}

void VehicleRecords_826AEF3C(rex::ppc::Register& r29) {
    (void)r29;
    SDL_Log("VehicleRecords_826AEF3C called - stub");
}

void VehicleRecords_826AF6F8(rex::ppc::Register& r5) {
    (void)r5;
    SDL_Log("VehicleRecords_826AF6F8 called - stub");
}

void VehicleRecords_826B01B0(rex::ppc::Register& r8) {
    (void)r8;
    SDL_Log("VehicleRecords_826B01B0 called - stub");
}

void VehicleRecords_8279ABF0(rex::ppc::Register& r8) {
    (void)r8;
    SDL_Log("VehicleRecords_8279ABF0 called - stub");
}

