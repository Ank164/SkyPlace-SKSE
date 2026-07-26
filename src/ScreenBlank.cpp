#include "ScreenBlank.h"
#include "Graphics.h"
#include "Menu.h"

void ScreenBlank::Blank(float time, float fade) {
    float now = RE::Calendar::GetSingleton()->GetHoursPassed();
    endTime = now + time;
    fadeEnd = endTime + fade;
}

void ScreenBlank::Render() {
    float currentTime = RE::Calendar::GetSingleton()->GetHoursPassed();
    if (!Menu::IsOpen()) {
        if (currentTime < endTime) {
            auto drawList = ImGui::GetForegroundDrawList();
            ImVec2 screenMin = ImGui::GetMainViewport()->Pos;
            ImVec2 screenMax = ImVec2(screenMin.x + ImGui::GetMainViewport()->Size.x, screenMin.y + ImGui::GetMainViewport()->Size.y);
            drawList->AddRectFilled(screenMin, screenMax, IM_COL32(0, 0, 0, 255));
        } else if (currentTime < fadeEnd) {
            float fadeRatio = (fadeEnd - currentTime) / (fadeEnd - endTime);
            int alpha = static_cast<int>(fadeRatio * 255.0f);
            auto drawList = ImGui::GetForegroundDrawList();
            ImVec2 screenMin = ImGui::GetMainViewport()->Pos;
            ImVec2 screenMax = ImVec2(screenMin.x + ImGui::GetMainViewport()->Size.x, screenMin.y + ImGui::GetMainViewport()->Size.y);
            drawList->AddRectFilled(screenMin, screenMax, IM_COL32(0, 0, 0, alpha));
        }
    }
}

void ScreenBlank::Install() { Graphics::Register(Render); }

void ScreenBlank::Reset() {
    endTime = 0;
    fadeEnd = 0;
}