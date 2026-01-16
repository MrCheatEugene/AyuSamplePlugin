/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026
Follows GNU GPL v3 and Telegram Desktop licensing.
*/

#include "pch.h"
#include "AyuPlugin.h"
#include <history\history_item.h>

AyuPlugin _pluginInfo{
    L"Sample Plugin",
    L"A plugin, demonstrating how AyuGram can accept dynamic-library plugins, and do cool stuff with it.",
    L"AyuSamplePlugin.dll",
    true, // We are defining & using Shadow Filter functions
    NULL // NULL, because we're re-defining allocated memory space, yk?
}; // end structure

AyuPlugin* _ppluginInfo = &_pluginInfo; // static ptr

EXTERN_DLL_EXPORT AyuPlugin* pluginInfo() { // helper for external access
    return _ppluginInfo;
}

EXTERN_DLL_EXPORT InternalLoop internalLoop() {
    bool showedMemData = false;
    while (1) {
        if (_pluginInfo.memData.activeUserPtr != NULL && !showedMemData) {
            showedMemData = true;
            Beep(2000, 1000);
            char addrv[128];
            sprintf(addrv, "%p", _pluginInfo.memData.activeUserPtr);
            wchar_t a[128];
            mbstowcs(a, addrv, 128);
            MessageBox(NULL, a, L"MemData active User Ptr", MB_OK);
        }
        Sleep(1000);
    }
}

EXTERN_DLL_EXPORT InternalDoFilterHistoryItem doFilterHistoryItem(HistoryItem* HistoryItem) {
    return (InternalDoFilterHistoryItem)(HistoryItem->getText().text.contains("bannedWord"));
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE; // Return FALSE to reject plugin load.
}