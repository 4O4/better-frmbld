#pragma once

#include <Windows.h>

typedef const DWORD HOOKADDR;
typedef const size_t HOOKNOPS;

#define BETTER_FRMBLD_HOOK __declspec(naked) void
#define DECLARE_BETTER_FRMBLD_HOOK_N(funcname, startaddr, nops) \
	HOOKADDR funcname##HookStartAddr = startaddr; \
	HOOKNOPS funcname##HookNopsCount = nops; \
	void funcname();

#define DECLARE_BETTER_FRMBLD_HOOK(funcname, startaddr, endaddr) \
	HOOKADDR funcname##HookStartAddr = startaddr; \
	HOOKADDR funcname##HookEndAddr = endaddr; \
	void funcname();

#define DECLARE_BETTER_FRMBLD_HOOK_S(funcname, startaddr) \
	HOOKADDR funcname##HookStartAddr = startaddr; \
	HOOKADDR funcname##HookEndAddr = startaddr + 5; \
	void funcname();