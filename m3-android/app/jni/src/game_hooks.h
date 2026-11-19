// MCLA - Game Hook Stubs Header
#pragma once

#include <rex/ppc.h>

namespace mcla {

// Function hooks for game-specific patches
bool Hook_BrowserCreateGameVeto(rex::ppc::Register& r27);
bool Hook_CarbonPartPop(rex::ppc::Register& r1, rex::ppc::Register& r27, rex::ppc::Register& r31);
bool Hook_CarbonPartPush(rex::ppc::Register& r1, rex::ppc::Register& r27, rex::ppc::Register& r31);
bool Hook_CarbonToggle(rex::ppc::Register& r11);
bool Hook_ControlCaptureKey(rex::ppc::Register& r11, rex::ppc::Register& r28);
bool Hook_ControlReleaseKey(rex::ppc::Register& r11, rex::ppc::Register& r28);
bool Hook_GameSessionBegin(rex::ppc::Register& r1);
bool Hook_GameSessionEnd(rex::ppc::Register& r1);
bool Hook_GameCreateSession(rex::ppc::Register& r1);
bool Hook_GameCloseSession(rex::ppc::Register& r1);
bool Hook_GameJoinSession(rex::ppc::Register& r1, rex::ppc::Register& r2);
bool Hook_GameLeaveSession(rex::ppc::Register& r1);
bool Hook_GameGetSession(rex::ppc::Register& r1, rex::ppc::Register& r2);

} // namespace mcla