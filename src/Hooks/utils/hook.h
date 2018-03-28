#pragma once

#include <windows.h>

#define TAKE_REGISTERS_SNAPSHOT(dest) \
	__asm mov dest.eax, eax \
	__asm mov dest.ebx, ebx \
	__asm mov dest.ecx, ecx \
	__asm mov dest.edx, edx \
	__asm mov dest.ebp, ebp \
	__asm mov dest.esp, esp \
	__asm mov dest.esi, esi \
	__asm mov dest.edi, edi \
	__asm pushfd __asm pop dest.efl /* is this safe? */

#define ESP_ADFD esp + 0x24
#define ESP_AD esp + 0x20

typedef DWORD RETADDR;

typedef const DWORD CDWORD;

typedef struct RegistersSnapshot {
	DWORD eax, ebx, ecx, edx, ebp, esp, esi, edi, eip, efl;
} RegistersSnapshot;

