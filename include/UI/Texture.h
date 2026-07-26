#pragma once

#include <d3d11.h>
#include "imgui.h"

class TextureManager{
private:
    static inline std::map<std::string, ImTextureID> textures;

public:
    static void Render(std::string path, ImVec2 position, ImVec2 size, ImColor color);
    static void Init(ID3D11Device* device, ID3D11DeviceContext* context);
};

