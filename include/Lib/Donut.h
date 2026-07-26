#pragma once
#include <numbers>
#include "imgui.h"
namespace Donut {

    void Draw(ImVec2 center, float radius_outer, float radius_inner, int slices, float coverage, ImU32 color);
}