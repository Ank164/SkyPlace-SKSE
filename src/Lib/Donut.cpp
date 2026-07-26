#include "Donut.h"

void Donut::Draw(ImVec2 center, float radius_outer, float radius_inner, int slices, float coverage, ImU32 color) {
    auto draw_list = ImGui::GetForegroundDrawList();

    coverage = std::clamp(coverage, 0.0f, 1.0f);
    int covered_slices = static_cast<int>(slices * coverage);

    const float angle_step = 2.0f * std::numbers::pi_v<float> / slices;
    const float epsilon = angle_step * 0.05f;

    for (int i = 0; i < covered_slices; ++i) {
        float angle0 = i * angle_step - epsilon;
        float angle1 = (i + 1) * angle_step + epsilon;

        ImVec2 p0_outer = ImVec2(center.x + cosf(angle0) * radius_outer, center.y + sinf(angle0) * radius_outer);
        ImVec2 p1_outer = ImVec2(center.x + cosf(angle1) * radius_outer, center.y + sinf(angle1) * radius_outer);
        ImVec2 p1_inner = ImVec2(center.x + cosf(angle1) * radius_inner, center.y + sinf(angle1) * radius_inner);
        ImVec2 p0_inner = ImVec2(center.x + cosf(angle0) * radius_inner, center.y + sinf(angle0) * radius_inner);

        draw_list->AddQuadFilled(p0_outer, p1_outer, p1_inner, p0_inner, color);
    }
}

