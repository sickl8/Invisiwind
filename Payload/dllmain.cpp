// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <stdlib.h>
#include <TlHelp32.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <iostream>
#include <tchar.h>

struct handle_data {
    unsigned long process_id;
    HWND window_handle;
};

BOOL is_main_window(HWND handle)
{
    return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
    handle_data& data = *(handle_data*)lParam;
    unsigned long process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);
    if (data.process_id != process_id || !is_main_window(handle))
        return TRUE;
    data.window_handle = handle;
    return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
    handle_data data;
    data.process_id = process_id;
    data.window_handle = 0;
    EnumWindows(enum_windows_callback, (LPARAM)&data);
    return data.window_handle;
}

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

BOOL CALLBACK EnumThreadWndProc1(_In_ HWND hwnd, _In_ LPARAM lParam) {
    if (!SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE)) {
        std::string errstring = GetLastErrorAsString();
        if (!errstring.empty()) {
            wchar_t p[200];
            mbstowcs_s(NULL, p, errstring.c_str(), 199);
            MessageBox(
                NULL,
                (LPCWSTR)p,
                (LPCWSTR)L"setwindowdisplayaffinity error",
                MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
            );
        }
    }
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
);

HMODULE dllmainhmodule;

void dllsetwda() {
    DWORD tid;
    DWORD pid = GetCurrentProcessId();
    tid = EnumerateThreads(pid);
    int i = 0;
    while (tid) {
        EnumThreadWindows(tid, EnumThreadWndProc1, NULL);
        tid = EnumerateThreads(pid);
        i++;
    }
    HMODULE hModule = NULL;
    hModule = dllmainhmodule;
    if (hModule)
    {
        FreeLibraryAndExitThread(hModule, 0);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
    dllmainhmodule = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        dllsetwda();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return FALSE;
}

