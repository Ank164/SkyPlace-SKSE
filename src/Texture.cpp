#include "Texture.h"

#include <vector>

#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/DDSTextureLoader.h"

#include <string>
#include <codecvt>
#include <locale>
#include <vector>

class TextureLoader {
    static inline ID3D11Device* device = NULL;
    static inline ID3D11DeviceContext* context = NULL;

public:
    static void Init(ID3D11Device* device, ID3D11DeviceContext* context);
    static ID3D11ShaderResourceView* LoadTextureFromDDSFile(std::string path);
    static ID3D11ShaderResourceView* LoadTextureFromWICFile(std::string path);
};




namespace File {
    inline bool Exists(const wchar_t* filename) {
        std::ifstream file(filename);
        return file.good();
    }
}

ImTextureID LoadDDS(std::string imagePath) {
    auto textureId = TextureLoader::LoadTextureFromDDSFile(imagePath);
    return reinterpret_cast<ImTextureID>(textureId);
}

ImTextureID LoadWIC(std::string imagePath) {
    auto textureId = TextureLoader::LoadTextureFromWICFile(imagePath);
    return reinterpret_cast<ImTextureID>(textureId);
}

const wchar_t* convertToWChar(const std::string& str) {
    static std::vector<wchar_t> wideStr;
    wideStr.clear();

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(str);
    wideStr.assign(wide.begin(), wide.end());
    wideStr.push_back(L'\0');
    return wideStr.data();
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

ImTextureID LoadTextureAny(std::string imagePath) {
    if (endsWith(imagePath, ".dds")){
        return LoadDDS(imagePath);
    }
    return LoadWIC(imagePath);
}

void TextureManager::Render(std::string texturePath, ImVec2 position, ImVec2 size, ImColor color) {

    auto it = textures.find(texturePath);

    ImTextureID textureId;
    if (it != textures.end()) {
        textureId = it->second;
    } else {
        textureId = LoadTextureAny(texturePath);
        textures[texturePath] = textureId;
    }

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    draw_list->AddImage(textureId, position, {position.x+size.x, position.y+size.y}, {0, 0}, {1, 1}, color);
}

void TextureManager::Init(ID3D11Device* device, ID3D11DeviceContext* context) 
{ TextureLoader::Init(device, context); }

void TextureLoader::Init(ID3D11Device* a_device, ID3D11DeviceContext* a_context) {
    device = a_device;
    context = a_context;
}

ID3D11ShaderResourceView* TextureLoader::LoadTextureFromDDSFile(std::string path) {
    if (!device || !context) return NULL;
    auto wpath = convertToWChar(path);

    if (!File::Exists(wpath)) {
        return NULL;
    }

    ID3D11ShaderResourceView* texture;


    DirectX::CreateDDSTextureFromFile(device, context, wpath, nullptr, &texture);

    return texture;
}

ID3D11ShaderResourceView* TextureLoader::LoadTextureFromWICFile(std::string path) {
    if (!device || !context) return NULL;
    auto wpath = convertToWChar(path);

    if (!File::Exists(wpath)) {
        return NULL;
    }

    ID3D11ShaderResourceView* texture;
    DirectX::CreateWICTextureFromFile(device, context, wpath, nullptr, &texture);

    return texture;
}
