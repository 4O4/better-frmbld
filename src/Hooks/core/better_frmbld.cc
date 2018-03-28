// Patches.cpp : Defines the exported functions for the DLL application.
//

#include "core/stdafx.h"
#include "core/better_frmbld.h"
#include "core/all_hooks.h"
#include "utils/memory.h"

#define INSTALL_HOOK_N(name) utils::memory::InstallHook(name, name##HookStartAddr, (size_t) name##HookNopsCount);
#define INSTALL_HOOK(name) utils::memory::InstallHook(name, name##HookStartAddr, (DWORD) name##HookEndAddr);

BETTER_FRMBLD_API void Init()
{
	INSTALL_HOOK(hooks::gui_controls::BypassBlueColorRefs);
	INSTALL_HOOK(hooks::gui_controls::BetterWindowTitle);
	INSTALL_HOOK(hooks::cmcs_support::LoadCodeEditorCtrlLib);

	utils::memory::FillWithNops((DWORD) 0x64CEA85B, (DWORD) 0x64CEA88C);
}

