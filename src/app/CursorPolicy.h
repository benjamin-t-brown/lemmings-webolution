#pragma once

#include <cstdint>
#include <string_view>

namespace webo::app {

// READ: software-cursor state numbers at 0x455341..0x45564e; binding
// 0x45620b..0x456323. State producers outside that decision are not guessed.
enum class CursorState : int {
    Hidden = 0, NoLemming = 1, NoLemmingLeft = 2, NoLemmingRight = 3,
    Lemming = 4, LemmingLeft = 5, LemmingRight = 6, Grab = 7,
    Exit = 8, Entrance = 9, MouseDown = 10
};

// Results of the native hit-test/filter stage, NOT a replacement for picking.
// In particular exitMarker1898 is a retained input, not inferred from a name
// or simply from an object hit. Its writer is outside the recovered decision.
struct CursorHits {
    bool lemming16d8;
    bool object1890;
    bool entrance1894;
    bool exitMarker1898;
};

// No gameplay defaults: callers must explicitly supply unresolved producers.
struct CursorPolicyInput {
    CursorState previous;
    int displayMode0c;
    int effectiveMode; // 0x45328c result, not assumed equal to +8
    int uiState4cc134;
    bool predicate55cb0;
    bool predicate55dd0;
    CursorHits hits;
    std::uint8_t filter16b1;
    std::uint8_t filter16b2;
    int panelIndex; // completed descending hit-test result: -1 or 0..9
    int panelAvailability; // [0x4c91d8 + index*4], ignored for -1
};

// READ, not live-validated. Includes grab's early return and the later panel
// override. Picking/geometry, cursor motion, and side effects stay with caller.
CursorState selectCursorState(const CursorPolicyInput& input);

// Software route only (byte [0x58cdad] == 0). The alternate native OS-cursor
// route uses different handles; this does not claim to implement that backend.
// Preserves native fallback: all unknown state numbers bind _curLemRight.
std::string_view softwareCursorResource(CursorState state);

} // namespace webo::app
