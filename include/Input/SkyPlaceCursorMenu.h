#pragma once

class SkyPlaceCursorMenu final : public RE::IMenu {
public:
    static constexpr std::string_view MENU_NAME = "SkyPlace Cursor Menu";

    SkyPlaceCursorMenu();

    void PostDisplay() override;

    static RE::IMenu* Create();
    static void Register();
    static void SetOpen(bool value);
};
