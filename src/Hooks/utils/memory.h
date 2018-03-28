#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace utils::memory {
	// Creates a call for the given hook function at the given destination address.
	// Optionally after the call several NOPs can be inserted to overwrite damaged opcodes / functions.
	// Parameters:
	// -> destAddr: The address where the hook call should be performed.
	// -> func: The hook function to be called.
	// -> nopCount: The count of the NOPs to be inserted after the 5 CALL bytes.
	void InstallHook(void(*func)(void), DWORD destAddr, size_t nopCount);

	void InstallHook(void(*func)(void), DWORD destAddr, DWORD endAddr);

	void InstallHook(void(*func)(void), DWORD destAddr);

	// Copies the given data to a given address.
	// Parameters:
	// -> destAddr: The address where the data should be copied to.
	// -> data: The pointer to the data.
	// -> byteCount: The length of the data to be copied (byte count).
	void SafeWrite(DWORD destAddr, void *data, size_t byteCount);	

	void FillWithNops(DWORD destAddr, size_t count);

	void FillWithNops(DWORD destAddr, DWORD endAddr);
}