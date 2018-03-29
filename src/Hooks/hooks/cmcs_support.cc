#include "hooks/cmcs_support.h"
#include "utils/hook.h"
#include "utils/filesystem.h"
#include "vendor/inih/INIReader.h"

namespace hooks::cmcs_support {
	using utils::filesystem::FileExists;
	bool IsUnicodeEnv();
	void ReloadDllPathsConfig();

	LPCSTR cmcsUnicodeDllPath = "CMCS100U.DLL";
	LPCSTR cmcsNonUnicodeDllPath = "CMCS100.DLL";
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

		ReloadDllPathsConfig();

		if (IsUnicodeEnv()) {
			if (FileExists(cmcsUnicodeDllPath)) {
				unicodeDllPathAddr = reinterpret_cast<DWORD>(cmcsUnicodeDllPath);
			}

			finalDllPathAddr = unicodeDllPathAddr;
		}
		else {
			if (FileExists(cmcsNonUnicodeDllPath)) {
				nonUnicodeDllPathAddr = reinterpret_cast<DWORD>(cmcsNonUnicodeDllPath);
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

	void ReloadDllPathsConfig() {
		char exepath[MAX_PATH + 1] = { 0 };
		GetModuleFileNameA(NULL, exepath, MAX_PATH);

		std::string inipath(exepath);
		inipath.erase(inipath.rfind('\\'));
		inipath.append("\\better-frmbld.ini");

		INIReader reader(inipath.c_str());

		if (reader.ParseError() >= 0) {
			cmcsUnicodeDllPath = reader.Get("cmcs_support", "unicode_dll_path", cmcsUnicodeDllPath).c_str();
			cmcsNonUnicodeDllPath = reader.Get("cmcs_support", "non_unicode_dll_path", cmcsNonUnicodeDllPath).c_str();
		}
	}
}