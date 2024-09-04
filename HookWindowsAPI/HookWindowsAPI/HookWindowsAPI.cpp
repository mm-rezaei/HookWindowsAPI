#define Hook_DLL_Name L"NtHookDLL.dll"

#include <iostream>
#include <windows.h>
#include <thread>
#include <TlHelp32.h>
#include <future>

#include "Notifier.h"
#include "SynchronizationPoint.h"
#include "NamedPipeServer.h"
#include "ProcessInjectionHelper.h"

using namespace std;

void PrintMessage(const wstring& message)
{
	DWORD writtenBytes;

	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), message.c_str(), DWORD(message.length()), &writtenBytes, NULL);

	wcout << endl;
}

void PrintErrorMessage(const wstring& message)
{
	DWORD writtenBytes;

	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), message.c_str(), DWORD(message.length()), &writtenBytes, NULL);

	wcout << endl;
	wcout << L"Error: Press any key to exit..." << endl;
}

bool PipeServerThread(Notifier& notifier, SynchronizationPoint& synchronizationPoint)
{
	NamedPipeServer& server = NamedPipeServer::Instance();

	if (server.Create(notifier, synchronizationPoint))
	{
		server.ReadMessages();
	}

	return true;
}

wstring GetDllFullPath(wstring& dllname)
{
	wchar_t buffer[MAX_PATH];

	DWORD length = GetCurrentDirectoryW(MAX_PATH, buffer);

	if (length != 0)
	{
		return wstring(buffer) + L"\\" + dllname;
	}

	return wstring(L"");
}

void HookWindowsApi(wstring& dllPath, wstring& processName)
{
	Notifier notifier;
	SynchronizationPoint synchronizationPoint;

	notifier.RegisterNotificationCallback(PrintMessage);
	notifier.RegisterErrorCallback(PrintErrorMessage);

	// Start the pipe server in a separate thread
	thread pipeThread(PipeServerThread, ref(notifier), ref(synchronizationPoint));

	synchronizationPoint.Wait();

	// Inject the DLL
	if (ProcessInjectionHelper::InjectDLL(processName, dllPath, notifier))
	{
		notifier.Notify(L"Press any key to stop...");

		cin.get();
	}
	else
	{
		notifier.NotifyError(L"DLL injection failed.");
	}

	//
	NamedPipeServer& pipeServer = NamedPipeServer::Instance();

	pipeServer.ClosePipe();
	pipeThread.join();

	//
	ProcessInjectionHelper::UnloadDLL(processName, dllPath, notifier);
}

int main()
{
	// Print new line
	wcout << endl;

	// Set the console output code page to UTF-8, and set the console mode to support wide characters
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

	// Expecting exactly argument: Process name
	auto cmdLine = GetCommandLineW();

	int argc;
	auto argv = CommandLineToArgvW(cmdLine, &argc);

	if (argc != 2)
	{
		wcout << L"Usage: " << argv[0] << " <Process Name>" << endl;
		wcout << L"Example: " << argv[0] << L" \"notepad.exe\"" << endl;

		return 1;
	}

	wstring dllName = Hook_DLL_Name;

	wstring dllPath = GetDllFullPath(dllName);
	wstring processName = wstring(argv[1]);

	HookWindowsApi(dllPath, processName);

	return 0;
}