#pragma once

class Screen {
public:
    static ImVec2 GetSize() { return ImGui::GetIO().DisplaySize; }
    static ImVec2 GetCenter() {
        auto size = GetSize();
        return {size.x / 2, size.y / 2};
    }
    static float Top(float pad) { return pad; }
    static float Left(float pad) { return pad; }
    static float Right(float pad) { return GetSize().x - pad; }
    static float Bottom(float pad) { return GetSize().y - pad; }
    static float CenterX(float pad) { return GetCenter().x + pad; }
    static float CenterY(float pad) { return GetCenter().y + pad; }
    static float Max(std::initializer_list<float> sizes) {
        auto result = 0;
        for (int n : sizes) {
            result = std::max(result, n);
        }
        return result;
    }

    static float AlignCenter(float pos, float size) { return pos - size / 2; }
    static float AlignEnd(float pos, float size) { return pos - size; }
    static ImVec2 AlignCenterX(ImVec2 pos, ImVec2 size) { return {AlignCenter(pos.x, size.x), pos.y}; }
    static ImVec2 AlignCenterY(ImVec2 pos, ImVec2 size) { return {pos.x, AlignCenter(pos.y, size.y)}; }
    static ImVec2 AlignEndX(ImVec2 pos, ImVec2 size) { return {AlignEnd(pos.x, size.x), pos.y}; }
    static ImVec2 AlignEndY(ImVec2 pos, ImVec2 size) { return {pos.x, AlignEnd(pos.y, size.y)}; }
    static ImVec2 Add(ImVec2 a, ImVec2 b) { return {a.x + b.x, a.y + b.y}; }

    static ImVec2 WorldToScreenLoc(RE::NiPoint3 position) {
        static uintptr_t g_worldToCamMatrix = RELOCATION_ID(519579, 406126).address();         // 2F4C910, 2FE75F0
        static auto g_viewPort = (RE::NiRect<float>*)RELOCATION_ID(519618, 406160).address();  // 2F4DED0, 2FE8B98

        ImVec2 screenLocOut;
        const RE::NiPoint3 niWorldLoc(position.x, position.y, position.z);

        float zVal;

        RE::NiCamera::WorldPtToScreenPt3((float(*)[4])g_worldToCamMatrix, *g_viewPort, niWorldLoc, screenLocOut.x,
                                         screenLocOut.y, zVal, 1e-5f);
        ImVec2 rect = ImGui::GetIO().DisplaySize;

        screenLocOut.x = rect.x * screenLocOut.x;
        screenLocOut.y = 1.0f - screenLocOut.y;
        screenLocOut.y = rect.y * screenLocOut.y;

        return screenLocOut;
    }
};
