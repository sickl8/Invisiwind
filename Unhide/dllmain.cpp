// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

void setDAForWindows() {
	HWND windowHandle = NULL;
	do {
		windowHandle = FindWindowEx(NULL, windowHandle, NULL, NULL);
		DWORD af;
		//if (GetWindowDisplayAffinity(windowHandle, &af)) {
		//}
		DWORD wlp = GetWindowLongPtr(windowHandle, GWL_STYLE);
		if ((wlp & WS_VISIBLE) && SetWindowDisplayAffinity(windowHandle, WDA_NONE)) {
			//const wchar_t* str = (af == WDA_NONE ? L"WDA_NONE" : L"WDA_EXCLUDEFROMCAPTURE");
			//MessageBox(
			//	NULL,
			//	(LPCWSTR)str,
			//	(LPCWSTR)L"getwindowdisplayaffinity",
			//	MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2
			//);
		}
	} while (windowHandle);
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

