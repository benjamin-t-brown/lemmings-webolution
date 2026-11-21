#pragma once

namespace webo::app {

// Provisional scheduling submits entrance and exit models before the tube in LegacyUnattributed.
template <class Primary, class LegacyUnattributed, class Tube, class Secondary>
void scheduleSceneFrame(Primary&& primary, LegacyUnattributed&& legacy,
                        Tube&& tube, Secondary&& secondary) {
  primary();
  legacy();
  tube();
  secondary();
}

// Receiver base Fluid follows the tube. At 0x441845 the order is tube,
// +0x160/+0x164, base Fluid, +0x15C, secondary.
template <class Primary, class LegacyUnattributed, class Tube, class Fluid, class Secondary>
void scheduleSceneFrame(Primary&& primary, LegacyUnattributed&& legacy,
                        Tube&& tube, Fluid&& fluid, Secondary&& secondary) {
  primary();
  legacy();
  tube();
  fluid();
  secondary();
}

}
