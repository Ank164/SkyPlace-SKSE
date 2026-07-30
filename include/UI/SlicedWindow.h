#pragma once

#include "imgui.h"

class SlicedWindow {
public:
    static bool Begin(
        const char* name,
        ImGuiWindowFlags flags = ImGuiWindowFlags_None);
    static void End();
};
