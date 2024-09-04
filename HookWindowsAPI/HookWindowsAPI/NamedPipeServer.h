#pragma once

#define PIPE_NAME L"\\\\.\\pipe\\NtHookDLLPipe"
#define PIPE_BUFFER_SIZE 1024 * 16

#include <windows.h>
#include <string>

#include "Notifier.h"

using namespace std;

class NamedPipeServer
{
public:

	NamedPipeServer(const NamedPipeServer&) = delete;

	NamedPipeServer& operator=(const NamedPipeServer&) = delete;

	static NamedPipeServer& Instance()
	{
		static NamedPipeServer instance(PIPE_NAME);

		return instance;
	}

	bool Create(Notifier& notifier, SynchronizationPoint& synchronizationPoint)
	{
		this->notifier = &notifier;

		pipeHandle = CreateNamedPipe(pipeName.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE, NMPWAIT_USE_DEFAULT_WAIT, NULL);

		if (pipeHandle == INVALID_HANDLE_VALUE)
		{
			NotifyError(L"CreateNamedPipe failed.");

			return false;
		}

		Notify(L"Waiting for a client to connect to the pipe...");

		synchronizationPoint.Signal();

		BOOL connectionResult = ConnectNamedPipe(pipeHandle, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (!connectionResult)
		{
			NotifyError(L"ConnectNamedPipe failed.");

			CloseHandle(pipeHandle);

			pipeHandle = INVALID_HANDLE_VALUE;

			return false;
		}

		Notify(L"Client connected to the pipe.");

		return true;
	}

	void ReadMessages()
	{
		wchar_t messageBuffer[1024];

		DWORD bytesRead;

		while (true)
		{
			BOOL readResult = ReadFile(pipeHandle, messageBuffer, sizeof(messageBuffer) - sizeof(wchar_t), &bytesRead, NULL);

			if (!readResult)
			{
				DWORD error = GetLastError();

				if (error == ERROR_BROKEN_PIPE || error == ERROR_OPERATION_ABORTED)
				{
					NotifyError(L"Pipe read operation stopped.");

					break;
				}
				else
				{
					NotifyError(L"ReadFile failed with error.");

					break;
				}
			}

			messageBuffer[bytesRead / sizeof(wchar_t)] = L'\0';

			Notify(messageBuffer);
		}
	}

	void ClosePipe()
	{
		if (pipeHandle != INVALID_HANDLE_VALUE)
		{
			CancelIoEx(pipeHandle, NULL);
		}
	}

private:

	NamedPipeServer(const wstring& pipeName) : pipeHandle(INVALID_HANDLE_VALUE), pipeName(pipeName), notifier(nullptr)
	{
	}

	~NamedPipeServer()
	{
		if (pipeHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(pipeHandle);
		}
	}

	void Notify(const wstring& message)
	{
		if (notifier != nullptr)
		{
			notifier->Notify(message);
		}
	}

	void NotifyError(const wstring& errorMessage)
	{
		if (notifier != nullptr)
		{
			notifier->NotifyError(errorMessage);
		}
	}

	HANDLE pipeHandle;
	wstring pipeName;
	Notifier* notifier;
};