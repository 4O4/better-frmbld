/*
* Injector based on injection.h by github.com/Janworks, but highly modified.
* Added support for unicode and changed memory modification logic to use
* pointers arithmetic instead of temporary counters. Also converted
* duplicated code to macros / lambads where possible
*/

#include "stdafx.h"
#include <string>
#pragma warning(disable:4996)

#define MEM_ADD_BYTE(loc, data) *loc++ = data;
#define MEM_ADD_DWORD(loc, data) memcpy(loc, data, sizeof(DWORD)); loc += sizeof(DWORD);
#define MEM_ADD_WSTR(loc, data) { \
		wchar_t** wcharAtLoc = (wchar_t**)&loc; \
		wcscpy(*wcharAtLoc, data); \
		*wcharAtLoc += wcslen(*wcharAtLoc) + 1; \
	}
#define MEM_ADD_STR(loc, data) { \
		char** charAtLoc = (char**)&loc; \
		strcpy(*charAtLoc, data); \
		*charAtLoc += strlen(*charAtLoc) + 1; \
	}

#define ASM_PUSH_LV 0x68
#define ASM_PUSH_LB 0x6A
#define ASM_CMP 0x83
#define ASM_CALL_EAX_1 0xFF
#define ASM_CALL_EAX_2 0xD0
#define ASM_JNZ 0x75
#define ASM_MOV_EAX_LV 0xB8

namespace injector {
	void InjectLib(HANDLE process, LPCWSTR dllPath, LPCSTR dllFunc)
	{
		// Path to DLL
		DWORD addrInjectDLLPath = 0;

		// DLL initialization function
		DWORD addrInjectDLLFunc = 0;

		// Error message box title
		DWORD addrInjectErrorTitle = 0;
		wchar_t injectErrorTitle[] = L"Error";

		// Error message: DLL loading failed
		DWORD addrInjectError1 = 0;
		wchar_t injectError1[] = L"Unable to inject library. Exiting.";

		// Error message: Initialization function call failed
		DWORD addrInjectError2 = 0;
		wchar_t injectError2[] = L"Unable to locate library init function. Exiting.";

		// Load kernel32.dll
		HMODULE kernel32 = LoadLibraryW(L"kernel32.dll");

		// Get addresses of some kernel32 functions
		FARPROC procLoadLibraryW = GetProcAddress(kernel32, "LoadLibraryW");
		FARPROC procGetProcAddress = GetProcAddress(kernel32, "GetProcAddress");
		FARPROC procExitProcess = GetProcAddress(kernel32, "ExitProcess");
		FARPROC procExitThread = GetProcAddress(kernel32, "ExitThread");

		// Load user32.dll
		HMODULE user32 = LoadLibraryW(L"user32.dll");

		// Get address of MessageBox function (for error messages)
		FARPROC procMessageBoxA = GetProcAddress(user32, "MessageBoxW");

		// Reserve memory for patch and allocate it in the target process
		const LPBYTE heapBase = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 2048);
		LPVOID memProcAddr = VirtualAllocEx(process, 0, 2048, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		const DWORD memProcAddrVal = PtrToUlong(memProcAddr);


		/*** DATA ***/

		LPBYTE currentHeapLoc = heapBase;

		auto mapCurrentHeapLocToVirtualMemAddress = [&, memProcAddr]() {
			return memProcAddrVal + (currentHeapLoc - heapBase);
		};

		// Write DLL name
		addrInjectDLLPath = mapCurrentHeapLocToVirtualMemAddress();
		MEM_ADD_WSTR(currentHeapLoc, dllPath);

		// Write initialization function name
		addrInjectDLLFunc = mapCurrentHeapLocToVirtualMemAddress();
		MEM_ADD_STR(currentHeapLoc, dllFunc);

		// Write error message title
		addrInjectErrorTitle = mapCurrentHeapLocToVirtualMemAddress();
		MEM_ADD_WSTR(currentHeapLoc, injectErrorTitle);

		// Write error message 1
		addrInjectError1 = mapCurrentHeapLocToVirtualMemAddress();
		MEM_ADD_WSTR(currentHeapLoc, injectError1);

		// Write error message 2
		addrInjectError2 = mapCurrentHeapLocToVirtualMemAddress();
		MEM_ADD_WSTR(currentHeapLoc, injectError2);

		MEM_ADD_BYTE(currentHeapLoc, 0xCC);
		MEM_ADD_BYTE(currentHeapLoc, 0xCC);
		MEM_ADD_BYTE(currentHeapLoc, 0xCC);

		// Code segment start
		DWORD addrExecBegin = mapCurrentHeapLocToVirtualMemAddress();


		/*** DLL LOAD CODE ***/
		// The following commands are assembled by hand.
		// They are separated from each other to keep the code clear.

		// push addr
		// -> Push DLL name for LoadLibraryA onto the stack
		//mem[memPos++] = 0x68;
		MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LV);
		MEM_ADD_DWORD(currentHeapLoc, &addrInjectDLLPath);
		//memcpy(mem + memPos, &, 4);

		// mov eax, addr
		// -> Put address of LoadLibraryA into EAX
		MEM_ADD_BYTE(currentHeapLoc, ASM_MOV_EAX_LV);
		MEM_ADD_DWORD(currentHeapLoc, &procLoadLibraryW);

		// call eax
		// -> Call LoadLibraryA
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_1);
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_2);

		// Error checking => When errors occur EAX will be 0
		{
			// cmp eax, 0
			// -> Are there errors?
			MEM_ADD_BYTE(currentHeapLoc, ASM_CMP);
			MEM_ADD_BYTE(currentHeapLoc, 0xF8);
			MEM_ADD_BYTE(currentHeapLoc, 0x00);

			// jnz eip+0x1E
			// -> If not, skip error code (30 bytes)
			MEM_ADD_BYTE(currentHeapLoc, ASM_JNZ);
			MEM_ADD_BYTE(currentHeapLoc, 0x1E);

			// push 0x10
			// -> Push message box icon ID (MB_ICONHAND) onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LB);
			MEM_ADD_BYTE(currentHeapLoc, 0x10);

			// push addr
			// -> Push address of error message title onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LV);
			MEM_ADD_DWORD(currentHeapLoc, &addrInjectErrorTitle);

			// push addr
			// -> Push address of error message onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LV);
			MEM_ADD_DWORD(currentHeapLoc, &addrInjectError1);

			// push 0
			// -> Push message box window handle onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LB);
			MEM_ADD_BYTE(currentHeapLoc, 0x00);

			// mov eax, addr
			// -> Put address of MessageBoxA into EAX
			MEM_ADD_BYTE(currentHeapLoc, ASM_MOV_EAX_LV);
			MEM_ADD_DWORD(currentHeapLoc, &procMessageBoxA);

			// call eax
			// -> Call MessageBoxA and show error message
			MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_1);
			MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_2);

			// push 0
			// -> Push error code for ExitProcess onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LB);
			MEM_ADD_BYTE(currentHeapLoc, 0x00);

			// mov eax, addr
			// -> Put address of ExitProcess into EAX
			MEM_ADD_BYTE(currentHeapLoc, ASM_MOV_EAX_LV);
			MEM_ADD_DWORD(currentHeapLoc, &procExitProcess);

			// call eax
			// -> Call ExitProcess, end of execution
			MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_1);
			MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_2);
		}

		// push addr
		// -> Push address of initialization function for GetProcAddress onto the stack
		MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LV);
		MEM_ADD_DWORD(currentHeapLoc, &addrInjectDLLFunc);

		// push eax
		// -> Push DLL handle onto the stack (still in EAX)
		MEM_ADD_BYTE(currentHeapLoc, 0x50);

		// mov eax, addr
		// -> Put address of GetProcAddress into EAX
		MEM_ADD_BYTE(currentHeapLoc, ASM_MOV_EAX_LV);
		MEM_ADD_DWORD(currentHeapLoc, &procGetProcAddress);

		// call eax
		// -> Call GetProcAddress
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_1);
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_2);

		// Error checking => When errors occur EAX will be 0
		{
			// cmp eax, 0
			// -> Are there errors?
			MEM_ADD_BYTE(currentHeapLoc, ASM_CMP);
			MEM_ADD_BYTE(currentHeapLoc, 0xF8);
			MEM_ADD_BYTE(currentHeapLoc, 0x00);

			// jnz eip+0x1E
			// -> If not, skip error code (28 bytes)
			MEM_ADD_BYTE(currentHeapLoc, ASM_JNZ);
			MEM_ADD_BYTE(currentHeapLoc, 0x1C);

			// push 0x10
			// -> Push message box icon ID (MB_ICONHAND) onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LB);
			MEM_ADD_BYTE(currentHeapLoc, 0x10);

			// push addr
			// -> Push address of error message title onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LV);
			MEM_ADD_DWORD(currentHeapLoc, &addrInjectErrorTitle);

			// push addr
			// -> Push address of error message onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LV);
			MEM_ADD_DWORD(currentHeapLoc, &addrInjectError2);

			// push 0
			// -> Push message box window handle onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LB);
			MEM_ADD_BYTE(currentHeapLoc, 0x00);

			// mov eax, addr
			// -> Put address of MessageBoxA into EAX
			MEM_ADD_BYTE(currentHeapLoc, ASM_MOV_EAX_LV);
			MEM_ADD_DWORD(currentHeapLoc, &procMessageBoxA);

			// call eax
			// -> Call MessageBoxA and show error message
			MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_1);
			MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_2);

			// push 0
			// -> Push error code for ExitProcess onto the stack
			MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LB);
			MEM_ADD_BYTE(currentHeapLoc, 0x00);

			// mov eax, addr
			// -> Put address of ExitProcess into EAX
			MEM_ADD_BYTE(currentHeapLoc, ASM_MOV_EAX_LV);
			MEM_ADD_DWORD(currentHeapLoc, &procExitProcess);
		}

		// call eax
		// Depending on value of EAX (see error checking):
		// -> Call ExitProcess, end of execution
		// -> Call initialization function (address should be in EAX if no errors occured)
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_1);
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_2);


		/*** END OF DLL LOAD CODE ***/
		// The DLL stays loaded, the loading thread is terminated.

		// push 0
		// -> Push error code for ExitThread onto the stack
		MEM_ADD_BYTE(currentHeapLoc, ASM_PUSH_LB);
		MEM_ADD_BYTE(currentHeapLoc, 0x00);

		// mov eax, addr
		// -> Put address of ExitThread into EAX
		MEM_ADD_BYTE(currentHeapLoc, ASM_MOV_EAX_LV);
		MEM_ADD_DWORD(currentHeapLoc, &procExitThread);

		// call eax
		// -> Call ExitThread, end of execution
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_1);
		MEM_ADD_BYTE(currentHeapLoc, ASM_CALL_EAX_2);


		/*** INJECT THE LOAD CODE ***/

		// Change page protection to inject our patch
		DWORD pageProtectOld = 0;
		VirtualProtectEx(process, memProcAddr, currentHeapLoc - heapBase, PAGE_EXECUTE_READWRITE, &pageProtectOld);

		// Write patch into process memory
		DWORD writtenByteCount = 0;
		WriteProcessMemory(process, memProcAddr, heapBase, currentHeapLoc - heapBase, &writtenByteCount);

		// Restore page protection
		VirtualProtectEx(process, memProcAddr, currentHeapLoc - heapBase, pageProtectOld, &pageProtectOld);

		// Flush the CPU's instruction cache to make sure it'll really execute our patch
		FlushInstructionCache(process, memProcAddr, currentHeapLoc - heapBase);

		// Free our local patch memory
		HeapFree(GetProcessHeap(), 0, heapBase);

		// Run DLL loading thread
		// The start address is the offset where our load code starts (addrExecBegin)
		HANDLE loaderThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)ULongToPtr(addrExecBegin), 0, 0, NULL);

		// Wait for the loading thread to exit
		WaitForSingleObject(loaderThread, INFINITE);

		// Free the memory allocated in the process
		VirtualFreeEx(process, memProcAddr, 0, MEM_RELEASE);
	}
}