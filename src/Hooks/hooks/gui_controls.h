#pragma once

#include "core/hook_commons.h"

namespace hooks::gui_controls {
	DECLARE_BETTER_FRMBLD_HOOK_S(BypassBlueColorRefs, 0x64CEDDF0);
	DECLARE_BETTER_FRMBLD_HOOK(BetterWindowTitle, 0x64D63221, 0x64D63227);
}