#pragma once
#include "SkyPlaceCursorMenu.h"
#include "SKSEMenuFramework.h"

class Menu {
public:
    static bool IsOpen() {
        RE::UI* ui = RE::UI::GetSingleton();
        return ui &&
            (!ui->IsMenuOpen(SkyPlaceCursorMenu::MENU_NAME) &&
            ui->IsMenuOpen(RE::CursorMenu::MENU_NAME)) ||
            SKSEMenuFramework::IsAnyBlockingWindowOpened();
    }
    static inline void Close() {
        RE::UI* uiManager = RE::UI::GetSingleton();

        if (const auto inventoryMenu = uiManager->GetMenu<RE::InventoryMenu>()) {
            RE::UIMessageQueue::GetSingleton()->AddMessage("InventoryMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
            RE::UIMessageQueue::GetSingleton()->AddMessage("TweenMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }

        if (const auto favoritesMenu = uiManager->GetMenu<RE::FavoritesMenu>()) {
            RE::UIMessageQueue::GetSingleton()->AddMessage("FavoritesMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }

        if (const auto containerMenu = uiManager->GetMenu<RE::ContainerMenu>()) {
            RE::UIMessageQueue::GetSingleton()->AddMessage("ContainerMenu", RE::UI_MESSAGE_TYPE::kHide, nullptr);
        }
    }
};
