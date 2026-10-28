#include "app/CursorPolicy.h"

namespace app = webo::app;

#include <array>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace {
using app::CursorState;
using app::CursorPolicyInput;
void expect(bool condition, const char* label) {
    if (!condition) {
        std::cerr << "cursorpolicy FAIL: " << label << '\n';
        std::exit(1);
    }
}
CursorPolicyInput base() {
    return {CursorState::MouseDown, 0, 0, 0, false, false,
            {false, false, false, false}, 0, 0, -1, 0};
}
void result(const CursorPolicyInput& in, CursorState state, std::string_view name,
            const char* label) {
    const auto actual = app::selectCursorState(in);
    expect(actual == state && app::softwareCursorResource(actual) == name, label);
}
} // namespace

int main() {
    // All supported native software bindings, including retained/externally set
    // hidden and mouse-down states. These names are byte-read registration data.
    constexpr std::array<std::string_view, 11> names = {
        "", "_curNoLem", "_curNoLemLeft", "_curNoLemRight", "_curLem",
        "_curLemLeft", "_curLemRight", "^curGrab", "_curExit", "_curEnt", "_curMseDwn"};
    for (int s = 0; s < 11; ++s) {
        auto in = base();
        in.previous = static_cast<CursorState>(s);
        in.displayMode0c = 3; // neither main nor panel decision runs
        result(in, static_cast<CursorState>(s), names[s], "retained state binding");
    }
    expect(app::softwareCursorResource(static_cast<CursorState>(-1)) == "_curLemRight",
           "unknown-state fallback");
    expect(app::softwareCursorResource(static_cast<CursorState>(11)) == "_curLemRight",
           "high-state fallback");

    // Exhaust both raw direction bytes, not merely bools, for both hit classes.
    for (unsigned b1 = 0; b1 < 256; ++b1) {
        for (unsigned b2 = 0; b2 < 256; ++b2) {
            for (bool actor : {false, true}) {
                auto in = base();
                in.filter16b1 = b1;
                in.filter16b2 = b2;
                in.hits.lemming16d8 = actor;
                const int s = (actor ? 4 : 1) + (b1 == b2 ? 0 : b2 == 0 ? 2 : 1);
                result(in, static_cast<CursorState>(s), names[s], "raw direction bytes");
            }
        }
    }
    auto in = base();
    result(in, CursorState::NoLemming, "_curNoLem", "no hover");
    in.hits.object1890 = true;
    result(in, CursorState::Lemming, "_curLem", "object without markers uses lem resource");
    in.hits.exitMarker1898 = true;
    result(in, CursorState::Exit, "_curExit", "exit marker");
    in.hits.lemming16d8 = true;
    in.hits.entrance1894 = true;
    result(in, CursorState::Entrance, "_curEnt", "entrance before exit and actor");
    in.hits.object1890 = false;
    in.hits.lemming16d8 = false;
    result(in, CursorState::NoLemming, "_curNoLem", "empty test before stale markers");

    in = base();
    in.effectiveMode = 2;
    in.hits = {true, true, true, true};
    in.panelIndex = 4;
    in.panelAvailability = 1;
    in.predicate55cb0 = in.predicate55dd0 = true;
    result(in, CursorState::Grab, "^curGrab", "grab bypasses populated panel and hover");
    in.predicate55dd0 = false;
    result(in, CursorState::Lemming, "_curLem", "panel overrides entrance");
    in.panelAvailability = 0;
    result(in, CursorState::Entrance, "_curEnt", "empty panel retains main result in mode 0");
    in.displayMode0c = 1;
    result(in, CursorState::NoLemming, "_curNoLem", "empty panel forces nolem in mode 1");
    in.uiState4cc134 = 2;
    result(in, CursorState::Entrance, "_curEnt", "ui state 2 skips panel override");

    for (int index = -1; index < 10; ++index) for (int availability : {-1, 0, 1, 2}) {
        in = base();
        in.displayMode0c = 1;
        in.panelIndex = index;
        in.panelAvailability = availability;
        const auto wanted = index >= 0 && availability > 0
            ? CursorState::Lemming : CursorState::NoLemming;
        result(in, wanted, names[static_cast<int>(wanted)], "every panel index/availability");
    }

    // Every gate combination, independently stated expected outcome.
    for (int display : {0, 1, 3}) for (int effective : {0, 2, 3})
    for (int ui : {0, 1, 2}) for (bool a : {false, true}) for (bool b : {false, true}) {
        in = base();
        in.displayMode0c = display;
        in.effectiveMode = effective;
        in.uiState4cc134 = ui;
        in.predicate55cb0 = a;
        in.predicate55dd0 = b;
        const bool refresh = display == 0 || effective == 2;
        auto wanted = refresh ? CursorState::NoLemming : CursorState::MouseDown;
        if (refresh && a && effective == 2 && b) wanted = CursorState::Grab;
        else if (ui < 2 && (display == 1 || effective == 2) && display == 1)
            wanted = CursorState::NoLemming;
        const int s = static_cast<int>(wanted);
        result(in, wanted, names[s], "refresh/grab/panel gates");
    }
    std::cout << "cursorpolicy: native state/resource and ordered branch cases passed\n";
}
