#include <windows.h>
#include <detours.h>
#include <string>

#include "NamedPipeClient.h"

using namespace std;

typedef HANDLE(WINAPI* CreateFileW_Trigger)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

CreateFileW_Trigger OriginalCreateFileW = nullptr;
NamedPipeClient& PipeClient = NamedPipeClient::Instance();

// Hooked version of CreateFileW
HANDLE WINAPI HookedCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	// Send the file path to the injector
	wstring message = L"Hooked CreateFileW API called to open '" + wstring(lpFileName) + L"'.";

	PipeClient.SendMessageW(message);

	// Call the original CreateFileW function
	return OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		if (PipeClient.Connect())
		{
			wstring message = L"Hello from the client!";

			PipeClient.SendMessage(message);
		}

		// Hook the CreateFileW function
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		OriginalCreateFileW = (CreateFileW_Trigger)DetourFindFunction("kernel32.dll", "CreateFileW");
		DetourAttach(&(PVOID&)OriginalCreateFileW, HookedCreateFileW);

		DetourTransactionCommit();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		// Unhook the CreateFileW function
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)OriginalCreateFileW, HookedCreateFileW);
		DetourTransactionCommit();
	}

	return TRUE;
}