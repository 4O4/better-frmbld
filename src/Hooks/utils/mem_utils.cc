#include "utils/mem_utils.h"

namespace mem_utils {
	void InstallHook(void(*func)(void), DWORD destAddr, size_t nopCount)
	{
		// Calculate relative jump offset excluding the 5 CALL bytes
		DWORD callOffset = (PtrToUlong(func) - destAddr) - 5;

		// Create patch for hook call:
		// call relAddr
		// -> Call function at relAddr bytes offset
		BYTE patch[5] = { 0xE8, 0x00, 0x00, 0x00, 0x00 }; // call relAddr
		memcpy(patch + 1, &callOffset, sizeof(DWORD));
		PatchMemory(destAddr, patch, 5);

		// If NOPs are required, create them
		if (nopCount > 0)
		{
			// Create array with nopCount size and fill it with NOPs
			BYTE *nops = new BYTE[nopCount];
			memset(nops, 0x90, nopCount);

			// Copy NOP array
			PatchMemory(destAddr + 5, nops, nopCount);

			// Free NOP array memory
			delete nops;
		}
	}

	void PatchMemory(DWORD destAddr, void *data, size_t byteCount)
	{
		// Change page protection to enable writing
		DWORD pageProtectOld = 0;
		VirtualProtect((void *)destAddr, byteCount, PAGE_EXECUTE_READWRITE, &pageProtectOld);

		// Copy data
		memcpy((void *)destAddr, data, byteCount);

		// Restore page protection
		VirtualProtect((void *)destAddr, byteCount, pageProtectOld, &pageProtectOld);
	}
}