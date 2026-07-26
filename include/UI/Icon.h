#pragma once

#include "imgui.h"
#include "Input.h"

class Icons {
public:
    static void Render(Input::Source device, uint32_t key, ImVec2 position, ImVec2 size,
                       ImColor color);
};