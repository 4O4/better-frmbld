#include "hooks/cmcs_support.h"
#include "utils/hook.h"
#include "utils/filesystem.h"

namespace hooks::cmcs_support {
	using utils::filesystem::FileExists;
	bool IsUnicodeEnv();

	LPCSTR kCmcsUnicodeDllPath = "CMCS\\CMCS100U.DLL";
	LPCSTR kCmcsNonUnicodeDllPath = "CMCS\\CMCS100.DLL";
	CDWORD kOcmax10uDllStrAddr = 0x64D82658; // UIW.64D82658
	CDWORD kOcmax10DllStrAddr = 0x64D8264C; // UIW.64D8264C

	LPSTR finalDllPath = 0;
	DWORD finalDllPathAddr = 0;
	DWORD unicodeDllPathAddr = kOcmax10uDllStrAddr;
	DWORD nonUnicodeDllPathAddr = kOcmax10DllStrAddr;

	RETADDR retAddr = 0;
	RegistersSnapshot reg;

	BETTER_FRMBLD_HOOK LoadCodeEditorCtrlLib() {
		__asm {
			pop retAddr;
		}

		TAKE_REGISTERS_SNAPSHOT(reg);

		if (IsUnicodeEnv()) {
			if (FileExists(kCmcsUnicodeDllPath)) {
				unicodeDllPathAddr = reinterpret_cast<DWORD>(kCmcsUnicodeDllPath);
			}

			finalDllPathAddr = unicodeDllPathAddr;
		}
		else {
			if (FileExists(kCmcsNonUnicodeDllPath)) {
				nonUnicodeDllPathAddr = reinterpret_cast<DWORD>(kCmcsNonUnicodeDllPath);
			}

			finalDllPathAddr = nonUnicodeDllPathAddr;
		}

		finalDllPath = reinterpret_cast<LPSTR>(finalDllPathAddr);
		LoadLibraryA(finalDllPath); // push eax; call LoadLibraryA;

		__asm {
			push retAddr;
			ret;
		}
	}

	bool IsUnicodeEnv()	{
		auto someFlag = reinterpret_cast<LPBYTE>(reg.eax + 0x1E1); // byte ptr[eax + 0x1E1]

		return (
			reg.ecx == reg.edi // cmp ecx, edi;
			||
			reg.ecx == 0x367 // cmp ecx, 0x367;
			||
			reg.ecx == 0x369 // cmp ecx, 0x369;
			)
			&&
			(*someFlag & 1) == 1; // test byte ptr[eax + 0x1E1], 0x01;
	}
}