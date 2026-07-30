#pragma once
#include "Controls.h"
class Picker {
    struct SelectedObject {
        RE::FormID formID;
        RE::ObjectRefHandle handle;
    };

    static inline std::vector<SelectedObject> selectedObjects;
    static inline RE::ObjectRefHandle lastHoverHandle;

public:
    static RE::ObjectRefHandle GetLastHoverHandle();
    static void SaveChangeEvent();
    static void MoveEvent();
    static void MoveEvent(const RE::ObjectRefHandle& handle);
    static void PickEvent();
    static void ToggleSelectionEvent();
    static void DeselectAllEvent();
    static void Tick();
    static bool PickObjects(const std::vector<RE::ObjectRefHandle>& handles);
    static bool IsSelected(const RE::ObjectRefHandle& handle);
    static bool IsLastHoverSelected();
    static std::size_t GetSelectionCount();
    static std::vector<RE::ObjectRefHandle> GetSelectedHandles();
    static void SetSelection(const std::vector<RE::ObjectRefHandle>& handles);
    static void RemoveReference(RE::FormID formID);
    static void HideSelectionHighlights();
    static void ShowSelectionHighlights();
    static void ShowPlacementHighlights(const RE::ObjectRefHandle& movingHandle);
    static void HidePlacementHighlight(const RE::ObjectRefHandle& movingHandle);
};
