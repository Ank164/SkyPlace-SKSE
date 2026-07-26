#include "SkyPlaceCursorMenu.h"

SkyPlaceCursorMenu::SkyPlaceCursorMenu() {
    menuFlags.set(
        RE::UI_MENU_FLAGS::kRendersUnderPauseMenu,
        RE::UI_MENU_FLAGS::kAdvancesUnderPauseMenu,
        RE::UI_MENU_FLAGS::kUsesCursor);
    inputContext = RE::UserEvents::INPUT_CONTEXT_ID::kNone;
    depthPriority = 11;
}

RE::IMenu* SkyPlaceCursorMenu::Create() {
    return new SkyPlaceCursorMenu();
}

void SkyPlaceCursorMenu::Register() {
    RE::UI* ui = RE::UI::GetSingleton();
    if (ui) {
        ui->Register(MENU_NAME, Create);
    }
}

void SkyPlaceCursorMenu::SetOpen(bool value) {
    RE::UIMessageQueue* queue = RE::UIMessageQueue::GetSingleton();
    if (queue) {
        queue->AddMessage(
            MENU_NAME,
            value ? RE::UI_MESSAGE_TYPE::kShow : RE::UI_MESSAGE_TYPE::kHide,
            nullptr);
    }
}
