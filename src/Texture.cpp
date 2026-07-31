#include "Texture.h"

#include <algorithm>
#include <cmath>
#include <codecvt>
#include <locale>
#include <string>
#include <vector>

#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "nanosvg.h"
#include "nanosvgrast.h"

class TextureLoader {
    static inline ID3D11Device* device = nullptr;
    static inline ID3D11DeviceContext* context = nullptr;

    static ID3D11ShaderResourceView* ReadSVG(NSVGimage* image, ImVec2 size);

public:
    static void Init(ID3D11Device* device, ID3D11DeviceContext* context);
    static ID3D11ShaderResourceView* LoadTextureFromDDSFile(const std::string& path);
    static ID3D11ShaderResourceView* LoadTextureFromWICFile(const std::string& path);
    static ID3D11ShaderResourceView* LoadTextureFromSVGFile(const std::string& path, ImVec2 size);
};

namespace {
    std::wstring ConvertToWString(const std::string& value) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        return converter.from_bytes(value);
    }

    bool EndsWith(const std::string& value, const std::string& suffix) {
        return suffix.size() <= value.size() &&
            value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    ImTextureID LoadDDS(const std::string& imagePath) {
        ID3D11ShaderResourceView* texture =
            TextureLoader::LoadTextureFromDDSFile(imagePath);
        return reinterpret_cast<ImTextureID>(texture);
    }

    ImTextureID LoadWIC(const std::string& imagePath) {
        ID3D11ShaderResourceView* texture =
            TextureLoader::LoadTextureFromWICFile(imagePath);
        return reinterpret_cast<ImTextureID>(texture);
    }

    ImTextureID LoadSVG(const std::string& imagePath, ImVec2 size) {
        ID3D11ShaderResourceView* texture =
            TextureLoader::LoadTextureFromSVGFile(imagePath, size);
        return reinterpret_cast<ImTextureID>(texture);
    }

    ImTextureID LoadTextureAny(const std::string& imagePath, ImVec2 size) {
        if (EndsWith(imagePath, ".dds")) {
            return LoadDDS(imagePath);
        }
        if (EndsWith(imagePath, ".svg")) {
            return LoadSVG(imagePath, size);
        }
        return LoadWIC(imagePath);
    }
}

ImTextureID TextureManager::GetTexture(std::string texturePath, ImVec2 size) {
    std::string textureKey = texturePath;
    if (EndsWith(texturePath, ".svg")) {
        constexpr float snapFactor = 8.0f;
        size.x = std::ceil(size.x / snapFactor) * snapFactor;
        size.y = std::ceil(size.y / snapFactor) * snapFactor;
        textureKey = std::format(
            "{}-{}-{}",
            texturePath,
            static_cast<int>(size.x),
            static_cast<int>(size.y));
    }

    const auto texture = textures.find(textureKey);
    if (texture != textures.end()) {
        return texture->second;
    }

    const ImTextureID textureID = LoadTextureAny(texturePath, size);
    if (textureID) {
        textures[textureKey] = textureID;
    }
    return textureID;
}

void TextureManager::Render(
    std::string texturePath,
    ImVec2 position,
    ImVec2 size,
    ImColor color) {
    const ImTextureID textureID = GetTexture(texturePath, size);
    if (!textureID) {
        return;
    }

    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    drawList->AddImage(
        textureID,
        position,
        {position.x + size.x, position.y + size.y},
        {0.0f, 0.0f},
        {1.0f, 1.0f},
        color);
}

void TextureManager::Init(
    ID3D11Device* device,
    ID3D11DeviceContext* context) {
    TextureLoader::Init(device, context);
}

void TextureLoader::Init(
    ID3D11Device* newDevice,
    ID3D11DeviceContext* newContext) {
    device = newDevice;
    context = newContext;
}

ID3D11ShaderResourceView* TextureLoader::LoadTextureFromDDSFile(
    const std::string& path) {
    if (!device || !context) {
        return nullptr;
    }

    const std::wstring widePath = ConvertToWString(path);
    ID3D11ShaderResourceView* texture = nullptr;
    const HRESULT result = DirectX::CreateDDSTextureFromFile(
        device,
        context,
        widePath.c_str(),
        nullptr,
        &texture);
    return SUCCEEDED(result) ? texture : nullptr;
}

ID3D11ShaderResourceView* TextureLoader::LoadTextureFromWICFile(
    const std::string& path) {
    if (!device || !context) {
        return nullptr;
    }

    const std::wstring widePath = ConvertToWString(path);
    ID3D11ShaderResourceView* texture = nullptr;
    const HRESULT result = DirectX::CreateWICTextureFromFileEx(
        device,
        context,
        widePath.c_str(),
        0,
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE,
        0,
        0,
        DirectX::WIC_LOADER_IGNORE_SRGB,
        nullptr,
        &texture);
    return SUCCEEDED(result) ? texture : nullptr;
}

ID3D11ShaderResourceView* TextureLoader::LoadTextureFromSVGFile(
    const std::string& path,
    ImVec2 size) {
    NSVGimage* image = nsvgParseFromFile(path.c_str(), "px", 96.0f);
    return ReadSVG(image, size);
}

ID3D11ShaderResourceView* TextureLoader::ReadSVG(
    NSVGimage* image,
    ImVec2 size) {
    if (!image) {
        return nullptr;
    }
    if (!device) {
        nsvgDelete(image);
        return nullptr;
    }

    if (size.x <= 0.0f) {
        size.x = image->width;
    }
    if (size.y <= 0.0f) {
        size.y = image->height;
    }

    const int width = std::max(1, static_cast<int>(std::ceil(size.x)));
    const int height = std::max(1, static_cast<int>(std::ceil(size.y)));
    NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
    if (!rasterizer) {
        nsvgDelete(image);
        return nullptr;
    }

    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4);
    const float scale = std::min(
        size.x / image->width,
        size.y / image->height);
    const float offsetX = (static_cast<float>(width) - image->width * scale) * 0.5f;
    const float offsetY = (static_cast<float>(height) - image->height * scale) * 0.5f;
    nsvgRasterize(
        rasterizer,
        image,
        offsetX,
        offsetY,
        scale,
        pixels.data(),
        width,
        height,
        width * 4);
    nsvgDeleteRasterizer(rasterizer);
    nsvgDelete(image);

    D3D11_TEXTURE2D_DESC textureDescription{};
    textureDescription.Width = static_cast<UINT>(width);
    textureDescription.Height = static_cast<UINT>(height);
    textureDescription.MipLevels = 1;
    textureDescription.ArraySize = 1;
    textureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDescription.SampleDesc.Count = 1;
    textureDescription.Usage = D3D11_USAGE_DEFAULT;
    textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA textureData{};
    textureData.pSysMem = pixels.data();
    textureData.SysMemPitch = static_cast<UINT>(width * 4);

    ID3D11Texture2D* texture = nullptr;
    HRESULT result = device->CreateTexture2D(
        &textureDescription,
        &textureData,
        &texture);
    if (FAILED(result)) {
        return nullptr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC viewDescription{};
    viewDescription.Format = textureDescription.Format;
    viewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    viewDescription.Texture2D.MipLevels = 1;

    ID3D11ShaderResourceView* textureView = nullptr;
    result = device->CreateShaderResourceView(
        texture,
        &viewDescription,
        &textureView);
    texture->Release();
    return SUCCEEDED(result) ? textureView : nullptr;
}
