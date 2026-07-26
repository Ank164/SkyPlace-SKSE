#pragma once
#include "DrawDebug.h"

namespace DrawDebug {
    void DrawBoundingBox(
        const RE::ObjectRefHandle& handle,
        const glm::vec4& color = Color::Red);
    bool PointInBox(const RE::NiPoint3& point, RE::NiPoint3& from, RE::NiPoint3& to, RE::NiPoint3& center, RE::NiPoint3 euler);
    void DrawBox(RE::NiPoint3& from, RE::NiPoint3& to, RE::NiPoint3& center, RE::NiPoint3 euler, const glm::vec4& color = Color::Red);
    void DrawCube(RE::NiPoint3& center, float radius, RE::NiPoint3 euler, const glm::vec4& color = Color::Red);
    void DrawCube(RE::NiPoint3& center, float radius, const glm::vec4& color = Color::Red);

}
