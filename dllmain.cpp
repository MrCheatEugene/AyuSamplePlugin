/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026
Follows GNU GPL v3 and Telegram Desktop licensing.
*/

#include "pch.h"
#include "AyuPlugin.h"
#include <history\history.h>
#include <history\history_item.h>
#include <data\data_user.h>
AyuPlugin _pluginInfo{
    L"Sample Plugin",
    L"A plugin, demonstrating how AyuGram can accept dynamic-library plugins, and do cool stuff with it.",
    L"AyuSamplePlugin.dll",
    true, // We are defining & using Shadow Filter functions
    NULL // NULL, because we're re-defining allocated memory space, yk?
}; // end structure

AyuPlugin* _ppluginInfo = &_pluginInfo; // static ptr
EXTERN_DLL_EXPORT InternalPluginInfo pluginInfo() { // helper for external access
    return (InternalPluginInfo)_ppluginInfo;
}

EXTERN_DLL_EXPORT InternalLoop internalLoop() {
    bool showedMemData = false;
    while (1) {
        if (_pluginInfo.memData.activeUserPtr != NULL && !showedMemData) {
            showedMemData = true;
            try {
                UserData* auP = (UserData*)_pluginInfo.memData.activeUserPtr; // TODO: Figure out cross-thread QT access. Code below will not build, and if it does - QString is thread-specific. 
                //auto u = auP->username()
                //wchar_t a[128];
                //mbstowcs(a, u.c_str(), u.length()-1);
                //MessageBox(NULL, a, L"MemData active User Ptr", MB_OK);
            }
            catch (...) {
                MessageBox(NULL, L"Failed ptr usage", L"MemData active User Ptr", MB_OK);
            }
        }
        Sleep(1000);
    }
}

EXTERN_DLL_EXPORT InternalDoPreProcessMessage doPreProcessMessage(char* in, char* out) {
    // Example of a message override hook
    std::string i(in);
    i=std::regex_replace(i, std::regex("hello"), "replaced");
    strcpy(out, i.c_str());
    out[4096] = '\0';
}

DWORD64 GetIdleTimeMs()
{
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);

    if (!GetLastInputInfo(&lii))
        return (DWORD64)-1;

    return GetTickCount64() - lii.dwTime;
}

EXTERN_DLL_EXPORT InternalIsOnline doReturnIsOnline()
{
    /*
        Example function that demonstrates Online hook
    */
    const DWORD64 IDLE_LIMIT_MS = 10 * 1000; // 10 seconds

    DWORD64 idleMs = GetIdleTimeMs();
    if (idleMs == (DWORD64)-1)
        return (InternalIsOnline)false;

    return (InternalIsOnline)(idleMs < IDLE_LIMIT_MS);
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