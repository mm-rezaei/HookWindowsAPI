#pragma once

#define PIPE_NAME L"\\\\.\\pipe\\NtHookDLLPipe"

#include <windows.h>
#include <string>

using namespace std;

class NamedPipeClient
{
public:

	NamedPipeClient(const NamedPipeClient&) = delete;

	NamedPipeClient& operator=(const NamedPipeClient&) = delete;

	static NamedPipeClient& Instance()
	{
		static NamedPipeClient instance(PIPE_NAME);

		return instance;
	}

	bool Connect()
	{
		pipeHandle = CreateFile(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (pipeHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		return true;
	}

	void SendMessage(wstring& message)
	{
		DWORD bytesWritten;

		WriteFile(pipeHandle, message.c_str(), DWORD(message.length() * sizeof(wchar_t)), &bytesWritten, NULL);
	}

private:

	NamedPipeClient(const wstring& pipeName) : pipeHandle(INVALID_HANDLE_VALUE), pipeName(pipeName)
	{
	}

	~NamedPipeClient()
	{
		if (pipeHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pipeHandle);
		}
	}

	HANDLE pipeHandle;
	wstring pipeName;
};