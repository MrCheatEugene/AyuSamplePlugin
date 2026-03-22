/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026
Follows GNU GPL v3 and Telegram Desktop licensing.
*/

#include "pch.h"
#include "AyuPlugin.h"
#include <history/history.h>
#include <history/history_item.h>
#include <data/data_forum.h>
#include <data/data_user.h>
#include <data/data_history_messages.h>
#include "core/application.h"
#include "data/data_peer_id.h"
#include "data/data_document.h"
#include "apiwrap.h"

#include <data/data_session.h>
#include "main/main_domain.h"
#include "main/main_account.h"
#include "data/data_changes.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"
#include "main/main_session.h"
#include "api/api_updates.h"
#include <stddef.h>
#include <scheme.h>
#include <string>

AyuPlugin _pluginInfo{
    L"Sample Plugin",
    L"A plugin, demonstrating how AyuGram can accept dynamic-library plugins, and do cool stuff with it.",
    L"AyuSamplePlugin.dll",
    NULL // NULL, because we're re-defining allocated memory space, yk?
}; // end structure

AyuPlugin* _ppluginInfo = &_pluginInfo; // static ptr
EXTERN_DLL_EXPORT InternalPluginInfo pluginInfo() { // helper for external access
    return (InternalPluginInfo)_ppluginInfo;
}

// is run every time a FilterContoller gets a message
EXTERN_DLL_EXPORT InternalDoFilterHistoryItem doFilterHistoryItem(HistoryItem* hi) {
    if (hi->getText().text.contains("nometa")) {
		return (InternalDoFilterHistoryItem)FilteredState::RejectFurtherFiltering; // allowlist messages with "nometa" in them, even if it could be filtered by other filters
    }
    return (InternalDoFilterHistoryItem)FilteredState::NoMatch; 
}

// will be run once by the Engine
EXTERN_DLL_EXPORT void internalSetup() {
    Core::Application* app = (Core::Application*)_pluginInfo.memData.applicationAddr; // we can get the Application from the memData ptr
    if (!app) {
        MessageBoxA(nullptr, "Error: 'app' pointer is null!", "Debug", MB_OK);
        return;
    }
    auto s = (Main::Session*)_pluginInfo.memData.activeSessionPtr; // session, and yeah we point to it instead of copying, since we are in a DLL damn it!
    if (!s) {
        MessageBoxA(nullptr, "Error: active session is null!", "Debug", MB_OK);
        return;
    }
	auto uname = s->user()->username().toStdString();  // and then get the active account's username, just to show we can access all of that stuff!
    MessageBoxA(nullptr, uname.c_str(), "Active Account username", MB_OK); 

    /*
    Sending a message example:
    Since we're doing some deep stuff, and sendMessage calls some animation bullshit that has to be on a main thread, 
    we cannot use our thread. (technically we can, it'll be more unstable that way and fail 50-90% of the time)

    So, we create a void function, pass our Application and Session ptr to it, and do the magic there.
    */
    auto x = [app,s] {

        PeerId peerID = s->userPeerId(); // a shortcut to saved messages (chat with your active account); you can get chat or channel, like: peerFromChannel(ChannelId(3871594897ULL)); 
        History* history = s->data().history(peerID); // Get the history 
        if (!history) { 
            MessageBoxA(nullptr, "Couldn't load history.", "Debug", MB_OK ); 
            return;
        }
        auto message = Api::MessageToSend(Api::SendAction(history)); // Construct a simple message, ...
        message.textWithTags = { "Hi from Plugin Engine! ✨", {} };  // ..then fill it up with actual content! textWithTags is a Struct, by the way, so we can do it in one line, yay!
		s->api().sendMessage(std::move(message)); // then we call the Active Session API, and send the message! (yeah it needs std::move)
    };
    _pluginInfo.memData.addToQueue(x); // after making the function, we call our AddToQueue function, that function will be executed ASAP!

    /*
    Profile Update example:
    We can get ApiWrap by calling session->api(), which's a child of MTP::Sender
    Don't use ->request: it's not exported in the lib-file. 
    Exporting it via DLLexport causes it and requests to be "copied" in places, where it shouldn't be, 
    which breaks core telegram functions (https://t.me/ayuplugg/10)

    Instead, to send a request, you can get a pointer to MTP::Instance (as well as on any MTP::Sender object), 
	and then call sendReq with the request you want to send! (in this case, account.updateProfile)

    sendReq will return a mtpRequestID (int) which you can probably use somewhere.
    */
    auto y = [app, s] {
        using MTPaccountupdateProfile = tl::boxed<MTPaccount_updateProfile>; // boxed is required here, because sendReq doesn't understand unboxed requests
        auto r = MTPaccountupdateProfile(
            MTP_flags(MTPaccountupdateProfile::Flag::f_about),
            MTP_string(),
            MTP_string(),
            MTP_string("Updated bio via @ayuplugg ✨")
        );
        s->api().instancePtr()->sendReq(
            r
        );
    };
    _pluginInfo.memData.addToQueue(y);

    /*
    Add a document to "Saved music" example
    Saved Messages should have a valid music file, as it's last message
    */
    auto z = [app, s] {
        PeerId peerID = s->userPeerId(); // a shortcut to saved messages (chat with your active account)
        History* history = s->data().history(peerID); // Get the history 
        if (!history) {
            MessageBoxA(nullptr, "Couldn't load history.", "Debug", MB_OK);
            return;
        }
        auto m = history->lastMessage();
        if (m == nullptr || (m!=nullptr && m->media() == nullptr)) { // if saved messages is empty/last message is not a music file
            MessageBoxA(nullptr, "Send a music file to saved messages first.", "Debug", MB_OK);
            return;
        }

        /*
        by the way, the division below is only for debugging purposes, you can do:
		s->api().instancePtr()->sendReq(MTPaccount_SaveMusic(MTP_flags(0), history->lastMessage()->media()->document()->mtpInput(), MTPInputDocument()));
		and it will work the same, issue is you'll be struggling for hours to figure out what's wrong, because the debugger will say "uhh issue is somewhere here, something's null, somewhere in lambdas"
        */
        auto i = s->api().instancePtr(); // get the MTP::Instance ptr
        auto mE = m->media()->document(); // the actual document
        auto mI = mE->mtpInput(); // convert to MTPInputDocument
        auto r = MTPaccount_SaveMusic(
            MTP_flags(0),
            mI,
            MTPInputDocument()
        ); // form the request
        
        i->sendReq(r); // send it!
    };
    _pluginInfo.memData.addToQueue(z);
}

// will be run in a loop by the Engine
EXTERN_DLL_EXPORT void internalLoop() { // InternalLoop = void
}

EXTERN_DLL_EXPORT void doPreProcessMessage(char* in, char* out) { // InternalDoPreProcessMessage = void (char* in, char* out) - allows you to modify the message text before it gets processed by Telegram, which means you can effectively change the content of messages! (but not media, for now at least)
    // Example of a message override hook
    // Soon to be deprecated, but it works for now.
    std::string i(in);
    i=std::regex_replace(i, std::regex("hello"), "replaced");
    strcpy(out, i.c_str());
    out[4096] = '\0';
}

EXTERN_DLL_EXPORT InternalExcludeDeletion doExcludeDeleted(HistoryItem* i) {
	return (InternalExcludeDeletion)i->from()->username().contains("pmrgt"); // exclude deletion of messages from accounts with "pmrgt" in their username, just as a demo
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
		Example function that demonstrates Online hook: 
        if the user hasn't done any input, we consider them offline, even if the Telegram window is NOT active.
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
    // DLLMain shouldn't be used for anything else, except simple licensing checks (if needed).
    // MemData will have INVALID values at the moment we load the plugin. 
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