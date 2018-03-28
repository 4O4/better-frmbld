#include "utils/memory.h"

namespace utils::memory {
	void InstallHook(void(*func)(void), DWORD destAddr, size_t nopCount)
	{
		InstallHook(func, destAddr, (DWORD)(destAddr + 5 + nopCount));
	}

	void InstallHook(void(*func)(void), DWORD destAddr, DWORD endAddr)
	{
		InstallHook(func, destAddr);

		// If NOPs are required, create them
		if (endAddr > destAddr + 5)
		{
			FillWithNops(destAddr + 5, endAddr);
		}
	}

	void InstallHook(void(*func)(void), DWORD destAddr)
	{
		// Calculate relative jump offset excluding the 5 CALL bytes
		DWORD callOffset = (PtrToUlong(func) - destAddr) - 5;

		// Create patch for hook call:
		// call relAddr
		// -> Call function at relAddr bytes offset
		BYTE patch[5] = { 0xE8, 0x00, 0x00, 0x00, 0x00 }; // call relAddr
		memcpy(patch + 1, &callOffset, sizeof(DWORD));
		SafeWrite(destAddr, patch, 5);
	}

	void SafeWrite(DWORD destAddr, void *data, size_t byteCount)
	{
		// Change page protection to enable writing
		DWORD pageProtectOld = 0;
		VirtualProtect((void *)destAddr, byteCount, PAGE_EXECUTE_READWRITE, &pageProtectOld);

		// Copy data
		memcpy((void *)destAddr, data, byteCount);

		// Restore page protection
		VirtualProtect((void *)destAddr, byteCount, pageProtectOld, &pageProtectOld);
	}

	void FillWithNops(DWORD destAddr, size_t count)
	{
		FillWithNops(destAddr, destAddr + count);
	}

	void FillWithNops(DWORD destAddr, DWORD endAddr)
	{
		if (endAddr <= destAddr) {
			return;
		}

		unsigned int count = endAddr - destAddr;

		// Create array with nopCount size and fill it with NOPs
		BYTE *nops = new BYTE[count];
		memset(nops, 0x90, count);

		// Copy NOP array
		SafeWrite(destAddr, nops, count);

		// Free NOP array memory
		delete nops;
	}
}