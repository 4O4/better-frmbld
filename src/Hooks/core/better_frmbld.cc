// Patches.cpp : Defines the exported functions for the DLL application.
//

#include "core/stdafx.h"
#include "core/better_frmbld.h"
#include "core/all_hooks.h"
#include "utils/mem_utils.h"

#define INSTALL_HOOK(name) mem_utils::InstallHook(name, name##HookAddr, name##HookNopsCount);

BETTER_FRMBLD_API void Init()
{
	INSTALL_HOOK(hooks::gui_controls::BypassBlueColorRefs);
	INSTALL_HOOK(hooks::cmcs_support::LoadCodeEditorCtrlLib);
}

