#pragma once
class ScreenBlank {
    static inline float endTime = 0;
    static inline float fadeEnd = 0;
    static void Render();
public:
    static void Blank(float time, float fade);
    static void Install();
    static void Reset();
};