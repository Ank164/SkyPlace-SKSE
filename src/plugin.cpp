#include "Logger.h"
#include "UI.h"
#include "Hooks.h"
#include "DrawDebugExtension.h"
#include "Graphics.h"
#include "HUD.h"
#include "Picker.h"
#include "Placer.h"
#include "FormsByModelPath.h"
#include "FormsById.h"
#include "Translations.h"
#include "InputConfig.h"
#include "ScreenLog.h"
#include "SkyPlaceConfig.h"
#include "SkyPlaceCursorMenu.h"

void OnMessage(SKSE::MessagingInterface::Message* message) {
#ifndef NDEBUG
    DrawDebug::OnMessage(message);
#endif

    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        SkyPlaceCursorMenu::Register();
        HUD::Install();
        FormsById::Install();
        ScreenLog::Install();
    }
    if (
        message->type == SKSE::MessagingInterface::kNewGame||
        message->type == SKSE::MessagingInterface::kPreLoadGame
    ) {
        Picker::SaveChangeEvent();
        Placer::SaveChangeEvent();
    }
}



SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    // SkyPlace installs seven 14-byte branches across its hook modules. Modern
    // CommonLib initializes the trampoline only once, so reserve the complete
    // pool before any module attempts to install a hook.
    SKSE::Init(skse, { .trampoline = true, .trampolineSize = 128 });
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    SetupLog();
    Translations::Install();
    SkyPlaceConfig::Load();
    logger::info("Plugin loaded");
    UI::Register();
    Hooks::Install();
    Graphics::Install();
    FormsByModelPath::Install();
    InputConfig::Install();
    return true;
}
