#pragma once

namespace webo::app {

// SOURCE scheduling for the supported scene path. Entrance/Exit model-holder
// contributions retain their former backdrop timing until their native timing
// is established; they are not reclassified as primary by exclusion.
template <class Primary, class LegacyUnattributed, class Tube, class Secondary>
void scheduleSceneFrame(Primary&& primary, LegacyUnattributed&& legacy,
                        Tube&& tube, Secondary&& secondary) {
  primary();
  legacy();
  tube();
  secondary();
}

// fluid-surface-impl-001: receiver base-`Fluid` pass between the tube and the secondary holder
// (frame order 0x441845: tube, +0x160/+0x164, base Fluid, +0x15C, secondary). The four-stage form
// above is unchanged for callers that carry no fluid.
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
