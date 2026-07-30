#include "SlicedWindow.h"

#include <algorithm>
#include <array>
#include <cstring>

#include "Texture.h"

namespace {
    struct SlicedTexture {
        const char* path;
        ImVec2 sourceSize;
        ImVec4 sourceSlice;
        ImVec4 destinationBorder;
    };

    struct SlicedWindowMetrics {
        float titleHeight = 0.0f;
        float panelBorder = 0.0f;
    };

    constexpr float titleHeight = 80.0f;
    constexpr float titleHorizontalBorder = 70.0f;
    constexpr float panelBorder = 60.0f;
    constexpr float windowPadding = 30.0f;

    const SlicedTexture panelTexture{
        "Data\\SKSE\\plugins\\SkyPlaceAssets\\background.svg",
        ImVec2{428.0f, 154.0f},
        ImVec4{40.0f, 40.0f, 40.0f, 40.0f},
        ImVec4{panelBorder, panelBorder, panelBorder, panelBorder}
    };

    const SlicedTexture titleTexture{
        "Data\\SKSE\\plugins\\SkyPlaceAssets\\title.svg",
        ImVec2{800.0f, 46.0f},
        ImVec4{40.0f, 23.0f, 40.0f, 23.0f},
        ImVec4{
            titleHorizontalBorder,
            40.0f,
            titleHorizontalBorder,
            40.0f}
    };

    std::array<float, 4> BuildAxis(
        float minimum,
        float maximum,
        float leading,
        float trailing) {
        const float size = std::max(0.0f, maximum - minimum);
        leading = std::clamp(leading, 0.0f, size * 0.5f);
        trailing = std::clamp(trailing, 0.0f, size - leading);
        return {
            minimum,
            minimum + leading,
            maximum - trailing,
            maximum};
    }

    std::array<float, 4> BuildSourceAxis(
        float size,
        float leading,
        float trailing) {
        leading = std::clamp(leading, 0.0f, size);
        trailing = std::clamp(trailing, 0.0f, size - leading);
        return {0.0f, leading, size - trailing, size};
    }

    void AddSlicedImage(
        ImDrawList* drawList,
        ImTextureID textureID,
        const ImVec2& minimum,
        const ImVec2& maximum,
        const SlicedTexture& texture,
        const ImVec4& destinationBorder) {
        if (!drawList || !textureID ||
            maximum.x <= minimum.x || maximum.y <= minimum.y) {
            return;
        }

        const std::array<float, 4> x = BuildAxis(
            minimum.x,
            maximum.x,
            destinationBorder.x,
            destinationBorder.z);
        const std::array<float, 4> y = BuildAxis(
            minimum.y,
            maximum.y,
            destinationBorder.y,
            destinationBorder.w);
        const std::array<float, 4> u = BuildSourceAxis(
            texture.sourceSize.x,
            texture.sourceSlice.x,
            texture.sourceSlice.z);
        const std::array<float, 4> v = BuildSourceAxis(
            texture.sourceSize.y,
            texture.sourceSlice.y,
            texture.sourceSlice.w);

        for (int row = 0; row < 3; ++row) {
            for (int column = 0; column < 3; ++column) {
                if (x[column + 1] <= x[column] ||
                    y[row + 1] <= y[row] ||
                    u[column + 1] <= u[column] ||
                    v[row + 1] <= v[row]) {
                    continue;
                }

                drawList->AddImage(
                    textureID,
                    ImVec2{x[column], y[row]},
                    ImVec2{x[column + 1], y[row + 1]},
                    ImVec2{
                        u[column] / texture.sourceSize.x,
                        v[row] / texture.sourceSize.y},
                    ImVec2{
                        u[column + 1] / texture.sourceSize.x,
                        v[row + 1] / texture.sourceSize.y});
            }
        }
    }

    SlicedWindowMetrics GetMetrics(const ImVec2& size, float scale) {
        SlicedWindowMetrics metrics;
        metrics.titleHeight = std::clamp(
            titleHeight * scale,
            titleTexture.sourceSize.y * scale,
            std::max(titleTexture.sourceSize.y * scale, size.y * 0.35f));

        const float panelHeight = std::max(
            0.0f,
            size.y - metrics.titleHeight);
        metrics.panelBorder = std::min({
            panelBorder * scale,
            size.x * 0.12f,
            panelHeight * 0.35f});
        return metrics;
    }

    SlicedWindowMetrics DrawCurrentWindow(float scale) {
        const ImVec2 position = ImGui::GetWindowPos();
        const ImVec2 size = ImGui::GetWindowSize();
        const ImVec2 maximum{
            position.x + size.x,
            position.y + size.y};
        const SlicedWindowMetrics metrics = GetMetrics(size, scale);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        const ImTextureID panelTextureID = TextureManager::GetTexture(
            panelTexture.path,
            ImVec2{
                panelTexture.sourceSize.x * scale,
                panelTexture.sourceSize.y * scale});
        const ImTextureID titleTextureID = TextureManager::GetTexture(
            titleTexture.path,
            ImVec2{
                titleTexture.sourceSize.x * scale,
                titleTexture.sourceSize.y * scale});

        const ImVec2 titleMaximum{
            maximum.x,
            std::min(maximum.y, position.y + metrics.titleHeight)};
        const ImVec2 panelMinimum{position.x, titleMaximum.y};
        if (maximum.y > panelMinimum.y) {
            AddSlicedImage(
                drawList,
                panelTextureID,
                panelMinimum,
                maximum,
                panelTexture,
                ImVec4{
                    metrics.panelBorder,
                    metrics.panelBorder,
                    metrics.panelBorder,
                    metrics.panelBorder});
        }

        AddSlicedImage(
            drawList,
            titleTextureID,
            position,
            titleMaximum,
            titleTexture,
            ImVec4{
                titleTexture.destinationBorder.x * scale,
                titleTexture.destinationBorder.y * scale,
                titleTexture.destinationBorder.z * scale,
                titleTexture.destinationBorder.w * scale});
        return metrics;
    }

    void DrawTitle(
        const char* title,
        const SlicedWindowMetrics& metrics) {
        if (!title || !title[0]) {
            return;
        }

        const char* titleEnd = std::strstr(title, "##");
        if (!titleEnd) {
            titleEnd = title + std::strlen(title);
        }
        if (titleEnd == title) {
            return;
        }

        const ImVec2 textSize = ImGui::CalcTextSize(title, titleEnd);
        const ImVec2 position = ImGui::GetWindowPos();
        const ImVec2 size = ImGui::GetWindowSize();
        const ImVec2 textPosition{
            position.x + std::max(0.0f, (size.x - textSize.x) * 0.5f),
            position.y + std::max(
                0.0f,
                (metrics.titleHeight - textSize.y) * 0.5f)};
        ImGui::GetWindowDrawList()->AddText(
            textPosition,
            ImGui::GetColorU32(ImGuiCol_Text),
            title,
            titleEnd);
    }
}

bool SlicedWindow::Begin(
    const char* name,
    ImGuiWindowFlags flags,
    float scale) {
    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleColor(
        ImGuiCol_WindowBg,
        ImVec4{0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::PushStyleColor(
        ImGuiCol_Border,
        ImVec4{0.0f, 0.0f, 0.0f, 0.0f});
    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2{windowPadding * scale, windowPadding * scale});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    const bool renderContent = ImGui::Begin(name, nullptr, flags);
    ImGui::SetWindowFontScale(scale);
    const SlicedWindowMetrics metrics = DrawCurrentWindow(scale);
    DrawTitle(name, metrics);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    if (renderContent) {
        const float bodyStart = metrics.titleHeight + metrics.panelBorder;
        if (ImGui::GetCursorPosY() < bodyStart) {
            ImGui::SetCursorPosY(bodyStart);
        }
    }
    return renderContent;
}

void SlicedWindow::End() {
    ImGui::End();
}
