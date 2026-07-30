#pragma once

class SkyPromptClient {
public:
    static void Install();
    static void SetIsEnabled(bool value);
    static bool GetIsEnabled();
    static bool OnInput(RE::InputEvent* event);
    static void ShowInventoryClone(
        const RE::ObjectRefHandle& handle,
        RE::TESBoundObject* item);
    static void ShowPick(const RE::ObjectRefHandle& handle);
    static void ShowPlace();
    static void EnsurePlace();
    static void HidePlace();
    static void HidePick();
    static void HideInventoryClone();
};
