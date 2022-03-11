// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdlib.h>
#include <TlHelp32.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <iostream>
#include <tchar.h>

DWORD EnumerateThreads(DWORD pid)
{// needs working for a simpler loop
	char szText[MAX_PATH];
	static BOOL bStarted;
	static HANDLE hSnapPro, hSnapThread;
	static LPPROCESSENTRY32 ppe32;
	static PTHREADENTRY32 pte32;
	if (!bStarted)
	{
		if (!bStarted)
		{
			bStarted++;
			pte32 = new THREADENTRY32;
			pte32->dwSize = sizeof(THREADENTRY32);
			hSnapThread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (!hSnapThread)
			{
				//FormatErrorMessage("GetLastError -> hSnapThread = CreateToolhelp32Snapshot\n", GetLastError());
				delete pte32;
				bStarted = 0;
				return 0;
			}
			if (Thread32First(hSnapThread, pte32))
			{
				do
				{
					if (pid == pte32->th32OwnerProcessID)
					{
						//wsprintf(szText, "__yes Thread32First pid: 0x%X - tid: 0x%X\n", pid, pte32->th32ThreadID);
						//OutputDebugString(szText);
						return pte32->th32ThreadID;
					}
				} while (Thread32Next(hSnapThread, pte32));
			}
			//else
				//FormatErrorMessage("GetLastError ->Thread32First\n", GetLastError());
		}
	}
	if (Thread32Next(hSnapThread, pte32))
	{
		do
		{
			if (pid == pte32->th32OwnerProcessID)
			{
				//wsprintf(szText, "__yes Thread32First pid: 0x%X - tid: 0x%X\n", pid, pte32->th32ThreadID);
				//OutputDebugString(szText);
				return pte32->th32ThreadID;
			}
		} while (Thread32Next(hSnapThread, pte32));
	}
	//else
	//    FormatErrorMessage("GetLastError ->Thread32First\n", GetLastError());
	CloseHandle(hSnapThread);
	bStarted = 0;
	delete pte32;
	//OutputDebugString("__finished EnumerateThreads\n");
	return 0;
}

std::string GetLastErrorAsString()
{
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string();
	}
	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
	std::string message(messageBuffer, size);
	LocalFree(messageBuffer);
	return message;
}



BOOL CALLBACK callbackfunc(_In_ HWND hwnd, _In_ LPARAM param) {
	int wlp = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (wlp & WS_VISIBLE) {
		if (!SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE)) {
			// error
			DWORD err = GetLastError();
			std::string errstring = GetLastErrorAsString();
			if (!errstring.empty()) {
				wchar_t p[500];
				wchar_t wcharerrstring[200];
				memset(p, 0, sizeof(p));
				memset(wcharerrstring, 0, sizeof(wcharerrstring));
				mbstowcs_s(NULL, wcharerrstring, errstring.c_str(), 199);
				wsprintf(p, L"window handle: %X, err number: %d\nerr string: %ls\n", hwnd, err, wcharerrstring);
				MessageBox(
					NULL,
					(LPCWSTR)p,
					(LPCWSTR)L"setwindowdisplayaffinity error",
					MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
				);
			}
		}
	}
	return TRUE;
}

void setDAForWindows() {
	int pid = GetCurrentProcessId();
	int tid = EnumerateThreads(pid);
	while (tid) {
		EnumThreadWindows(tid, callbackfunc, NULL);
		tid = EnumerateThreads(pid);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		setDAForWindows();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return FALSE;
}

