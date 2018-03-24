#pragma once

#include <Windows.h>

typedef const DWORD HOOKADDR;
typedef const size_t HOOKNOPS;

#define BETTER_FRMBLD_HOOK __declspec(naked) void
#define DECLARE_BETTER_FRMBLD_HOOK(funcname, hookaddr, nops) \
	HOOKADDR funcname##HookAddr = hookaddr; \
	HOOKNOPS funcname##HookNopsCount = nops; \
	void funcname();