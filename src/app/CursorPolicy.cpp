#include "CursorPolicy.h"

namespace webo::app {
namespace {
CursorState directional(const CursorPolicyInput& in, bool lemming) {
    // Native compares the bytes themselves, not just their truth values.
    if (in.filter16b2 == in.filter16b1)
        return lemming ? CursorState::Lemming : CursorState::NoLemming;
    if (in.filter16b2 == 0)
        return lemming ? CursorState::LemmingRight : CursorState::NoLemmingRight;
    return lemming ? CursorState::LemmingLeft : CursorState::NoLemmingLeft;
}
} // namespace

CursorState selectCursorState(const CursorPolicyInput& in) {
    auto state = in.previous;
    // 0x454d83..0x454da4: otherwise retain state until possible panel override.
    if (in.displayMode0c == 0 || in.effectiveMode == 2) {
        // 0x4552c4..0x455340: this return bypasses even the panel override.
        if (in.predicate55cb0 && in.effectiveMode == 2 && in.predicate55dd0)
            return CursorState::Grab;

        if (!in.hits.lemming16d8 && !in.hits.object1890)
            state = directional(in, false);
        else if (in.hits.entrance1894)
            state = CursorState::Entrance;
        else if (in.hits.exitMarker1898)
            state = CursorState::Exit;
        else
            state = directional(in, true);
    }

    // 0x455478..0x45564e. Panel hit clears hover pointers in the native caller,
    // but those side effects must not be used to re-run the earlier decision.
    if ((in.uiState4cc134 == 0 || in.uiState4cc134 == 1) &&
        (in.displayMode0c == 1 || in.effectiveMode == 2)) {
        if ((in.panelIndex == -1 || in.panelAvailability < 1) && in.panelIndex < 11) {
            if (in.displayMode0c == 1)
                state = CursorState::NoLemming;
        } else {
            state = CursorState::Lemming;
        }
    }
    return state;
}

std::string_view softwareCursorResource(CursorState state) {
    switch (state) {
    case CursorState::Hidden: return {};
    case CursorState::NoLemming: return "_curNoLem";
    case CursorState::NoLemmingLeft: return "_curNoLemLeft";
    case CursorState::NoLemmingRight: return "_curNoLemRight";
    case CursorState::Lemming: return "_curLem";
    case CursorState::LemmingLeft: return "_curLemLeft";
    case CursorState::Grab: return "^curGrab";
    case CursorState::Exit: return "_curExit";
    case CursorState::Entrance: return "_curEnt";
    case CursorState::MouseDown: return "_curMseDwn";
    default: return "_curLemRight"; // includes state 6, 0x456316..0x456323
    }
}
} // namespace webo::app
