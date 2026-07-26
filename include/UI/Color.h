#pragma once
class Color {
public:
    static inline RE::NiColorA Create(uint32_t color) {
        // Extracting RGB components from the hex value
        float red = ((color & 0xFF000000) >> 24) / 255.0f;
        float green = ((color & 0x00FF0000) >> 16) / 255.0f;
        float blue = ((color & 0x0000FF00) >> 8) / 255.0f;
        float alpha = (color & 0x000000FF) / 255.0f;
        return RE::NiColorA(red, green, blue, alpha);
    }
};