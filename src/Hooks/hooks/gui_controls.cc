#include "hooks/gui_controls.h"
#include "utils/hook_utils.h"
#include <string>

namespace hooks::gui_controls {
	CDWORD kBlueColorRef = 0x02FF0000;

	COLORREF buttonTextColor = 0;

	RETADDR retAddr = 0;

	BETTER_FRMBLD_HOOK BypassBlueColorRefs() {

		__asm {
			pop retAddr;

			// Overwritten bytes [1-4]
			{
				pop edi;
				pop esi;
				or eax, ecx;
			}

			pushad;
			pushfd;

			mov eax, [ESP_ADFD - 4];
			cmp eax, kBlueColorRef;
			jne cleanup;

			push COLOR_BTNTEXT;
			call GetSysColor;
			mov buttonTextColor, eax;
			or buttonTextColor, 0x02000000

			popfd;
			popad;
			mov eax, buttonTextColor;
			jmp end;

		cleanup:
			popfd;
			popad;

		end:
			// Overwritten bytes [5]
			{
				pop ebx;
			}

			push retAddr;
			ret;
		}
	}

	LPCWSTR windowTitle = 0;
	RegistersSnapshot reg2;
	RETADDR retAddr2 = 0;
	std::vector<LPWSTR> vec = {};

	bool StartsWith(LPWSTR a, LPCWSTR b)
	{
		if (wcsncmp(a, b, wcslen(b)) == 0) return 1;
		return 0;
	}

	LPCWSTR kTitleWithModulePattern = L"Oracle Forms Builder -";
	LPCWSTR kTitleSimplePattern = L"Oracle Forms Builder";

	LPCWSTR GetBetterTitle() {
		LPWSTR titleInMemory = reinterpret_cast<LPWSTR>(reg2.ecx);

		std::wstring finalTitle;
		if (StartsWith(titleInMemory, kTitleWithModulePattern)) {
			std::wstring module(titleInMemory + wcslen(kTitleWithModulePattern));
			const size_t lastBackslashPos = module.rfind('\\');

			if (lastBackslashPos != std::wstring::npos) {
				std::wstring moduleName(module.substr(lastBackslashPos + 1));
				module.erase(lastBackslashPos);
				finalTitle += moduleName + L" - ";
			}

			finalTitle += module;			
			finalTitle += L" - Better Forms Builder";
		}
		else if (StartsWith(titleInMemory, kTitleSimplePattern)) {
			finalTitle = L"Better Forms Builder";
		}
		else {
			finalTitle = titleInMemory;
		}

		swprintf_s(titleInMemory, finalTitle.length() + 1, L"%s", finalTitle.c_str());
		
		return titleInMemory;
	}

	BETTER_FRMBLD_HOOK BetterWindowTitle() {
		__asm {
			pop retAddr2;
		}

		TAKE_REGISTERS_SNAPSHOT(reg2);

		windowTitle = GetBetterTitle();

		__asm {
			push windowTitle;
			push retAddr2;
			ret;
		}
	}
}