// Launcher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "inject_lib.h"
#include <string>

using injector::InjectLib;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	wchar_t workdir[MAX_PATH + 1] = { 0 };
	GetCurrentDirectory(MAX_PATH, workdir);

	wchar_t exepath[MAX_PATH + 1] = { 0 };
	GetModuleFileName(NULL, exepath, MAX_PATH);

	// Parse executable parameters
	std::wstring cmdline(workdir);
	cmdline.append(L"\\frmbld.exe");

	for (int i = 1; i < __argc; ++i)
	{
		cmdline.append(L" ").append(__wargv[i]);
	}

	std::wstring dllpath(exepath);
	dllpath.erase(dllpath.rfind('\\'));
	dllpath.append(L"\\better-frmbld.dll");

	// Variables for process startup
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi = { 0 };

	// Pass size of STARTUPINFO structure
	si.cb = sizeof(STARTUPINFO);

	// Run EXE, but suspend immediately
	int res = CreateProcess(NULL, (LPWSTR) cmdline.c_str(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, workdir, &si, &pi);
	if (res == 0)
	{
		std::wstring message(L"Unable to launch Better Forms Builder. Couldn't run executable: ");
		message.append(cmdline);
		MessageBox(0, message.c_str(), L"Error", MB_ICONERROR);

		return -1;
	}
	
	// Let it bootstrap things for a short moment, won't work otherwise
	ResumeThread(pi.hThread);
	Sleep(500);
	SuspendThread(pi.hThread);

	// Perform DLL injection
	InjectLib(pi.hProcess, dllpath.c_str(), "Init");

	// We're done here, continue
	ResumeThread(pi.hThread);

#ifdef _DEBUG
	WaitForSingleObject(pi.hProcess, INFINITE);
#endif

	return 0;
}