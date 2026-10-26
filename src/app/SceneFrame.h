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

}
