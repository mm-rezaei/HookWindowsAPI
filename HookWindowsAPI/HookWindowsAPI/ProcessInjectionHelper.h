#pragma once

#include <windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <string>

#include "Notifier.h"

using namespace std;

class ProcessInjectionHelper
{
	static bool ExistFile(wstring& inFileAddress)
	{
		bool result = false;

		DWORD fileAttributes = GetFileAttributesW(inFileAddress.c_str());

		if (fileAttributes != INVALID_FILE_ATTRIBUTES)
		{
			result = true;
		}

		return result;
	}

	static DWORD GetProcessIdByName(wstring& processName)
	{
		DWORD result = 0;

		HANDLE snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (snapshotHandle == INVALID_HANDLE_VALUE)
		{
			return 0;
		}

		PROCESSENTRY32W processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32W);

		if (Process32FirstW(snapshotHandle, &processEntry))
		{
			do
			{
				if (processName == processEntry.szExeFile)
				{
					result = processEntry.th32ProcessID;

					break;
				}
			} while (Process32NextW(snapshotHandle, &processEntry));
		}

		CloseHandle(snapshotHandle);

		return result;
	}

	static HMODULE GetRemoteModuleHandle(HANDLE hProcess, const wstring& dllPath)
	{
		HMODULE hModules[1024];
		DWORD cbNeeded;

		// Get a list of all the modules in the target process
		if (EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &cbNeeded, LIST_MODULES_ALL))
		{
			for (unsigned int index = 0; index < (cbNeeded / sizeof(HMODULE)); ++index)
			{
				wchar_t szModName[MAX_PATH];

				// Get the full path to the module's file
				if (GetModuleFileNameExW(hProcess, hModules[index], szModName, sizeof(szModName) / sizeof(wchar_t)))
				{
					// Check if this is the module we want to unload
					if (dllPath == szModName)
					{
						return hModules[index];
					}
				}
			}
		}

		return NULL;
	}

	static bool UnloadDLL(HANDLE hProcess, wstring& dllPath, Notifier& notifier)
	{
		// Get the base address of the loaded DLL in the target process
		HMODULE hModule = GetRemoteModuleHandle(hProcess, dllPath);

		if (!hModule)
		{
			notifier.Notify(L"Failed to find module handle for the DLL in the target process.");

			return false;
		}

		// Get the address of FreeLibrary in kernel32.dll
		HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");

		if (!hKernel32)
		{
			notifier.NotifyError(L"Failed to get handle of 'Kernel32.dll'.");

			CloseHandle(hProcess);

			return false;
		}

		// Get the address of FreeLibrary in kernel32.dll
		FARPROC pFreeLibrary = GetProcAddress(hKernel32, "FreeLibrary");

		if (!pFreeLibrary)
		{
			notifier.NotifyError(L"Failed to get address of 'FreeLibrary'.");

			CloseHandle(hProcess);

			return false;
		}

		// Create a remote thread in the target process to unload the DLL
		HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFreeLibrary, hModule, 0, NULL);

		if (hThread == NULL)
		{
			notifier.NotifyError(L"Failed to create remote thread to unload DLL.");

			CloseHandle(hProcess);

			return false;
		}

		// Wait for the remote thread to finish
		WaitForSingleObject(hThread, INFINITE);

		CloseHandle(hThread);

		return true;
	}

public:

	static bool UnloadDLL(DWORD processID, wstring& dllPath, Notifier& notifier)
	{
		if (ExistFile(dllPath))
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

			if (hProcess == NULL)
			{
				notifier.NotifyError(L"Failed to open target process.");

				return false;
			}

			return UnloadDLL(hProcess, dllPath, notifier);
		}

		return false;
	}

	static bool UnloadDLL(wstring& processName, wstring& dllPath, Notifier& notifier)
	{
		if (ExistFile(dllPath))
		{
			DWORD processId = GetProcessIdByName(processName);

			if (processId != 0)
			{
				return UnloadDLL(processId, dllPath, notifier);
			}
			else
			{
				notifier.NotifyError(L"The process name was not found.");
			}
		}

		return false;
	}

	static bool InjectDLL(DWORD processID, wstring& dllPath, Notifier& notifier)
	{
		if (ExistFile(dllPath))
		{
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);

			if (hProcess == NULL)
			{
				notifier.NotifyError(L"Failed to open target process.");

				return false;
			}

			// Unload the dll if was loaded in the target process
			UnloadDLL(hProcess, dllPath, notifier);

			// Allocate memory in the target process for the DLL path
			DWORD dllPathSize = DWORD((dllPath.length() + 1) * sizeof(wchar_t));

			LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, dllPathSize, MEM_COMMIT, PAGE_READWRITE);

			if (pDllPath == NULL)
			{
				notifier.NotifyError(L"Failed to allocate memory in target process.");

				CloseHandle(hProcess);

				return false;
			}

			// Write the DLL path to the allocated memory
			if (!WriteProcessMemory(hProcess, pDllPath, (LPVOID)dllPath.c_str(), dllPathSize, NULL))
			{
				notifier.NotifyError(L"Failed to write DLL path to target process memory.");

				VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
				CloseHandle(hProcess);

				return false;
			}

			// Get the address of LoadLibraryA in kernel32.dll
			HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");

			if (!hKernel32)
			{
				notifier.NotifyError(L"Failed to get handle of 'Kernel32.dll'.");

				VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
				CloseHandle(hProcess);

				return false;
			}

			FARPROC pLoadLibraryW = GetProcAddress(hKernel32, "LoadLibraryW");

			if (!pLoadLibraryW)
			{
				notifier.NotifyError(L"Failed to get address of 'LoadLibraryW'.");

				VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
				CloseHandle(hProcess);

				return false;
			}

			// Create a remote thread in the target process to load the DLL
			HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibraryW, pDllPath, 0, NULL);

			if (hThread == NULL)
			{
				notifier.NotifyError(L"Failed to create remote thread.");

				VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
				CloseHandle(hProcess);

				return false;
			}

			// Wait for the remote thread to finish
			WaitForSingleObject(hThread, INFINITE);

			// Clean up
			VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
			CloseHandle(hThread);
			CloseHandle(hProcess);

			return true;
		}
		else
		{
			notifier.NotifyError(L"The dll file does not exist.");
		}

		return false;
	}

	static bool InjectDLL(wstring& processName, wstring& dllPath, Notifier& notifier)
	{
		bool result = false;

		DWORD processId = GetProcessIdByName(processName);

		if (processId != 0)
		{
			result = InjectDLL(processId, dllPath, notifier);
		}
		else
		{
			notifier.NotifyError(L"The process name was not found.");
		}

		return result;
	}
};

