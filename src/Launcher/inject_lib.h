#pragma once

namespace injector {
	// Loads the given DLL function into the given process and runs it in its context.
	// Parameters:
	// -> process: The process to inject the DLL into.
	// -> dllPath: The path to the DLL being injected.
	// -> dllFunc: The DLL initialization function to be run after successful injection.
	void InjectLib(HANDLE process, LPCWSTR dllPath, LPCSTR dllFunc);
}