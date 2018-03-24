#include "hooks/gui_controls.h"
#include "utils/hook_utils.h"

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
}