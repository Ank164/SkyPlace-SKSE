#pragma once
class Menu {
public:
    static inline bool IsOpen() {
        const auto ui = RE::UI::GetSingleton();
        if (ui->IsApplicationMenuOpen() || ui->IsItemMenuOpen() || ui->IsModalMenuOpen() || ui->GameIsPaused()) {
            return true;
        }
        return false;
    }
    static inline void Close() {
        auto* uiManager = RE::UI::GetSingleton();

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