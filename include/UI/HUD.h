#pragma once

enum MenuEvent {
    kPlacerClose,
    kPlacePickAccepted,
    kPlacePlaceAccepted,
    kPlaceTranslateUpDown,
    kPlaceTranslateLeftRight,
    kPlaceTranslateDepth,
    kPlaceSetRaycastDistance,
    kPlaceOrbitRotate,
    kPlaceScale,

    kClosePick,
    kPickPickAccepted,
    kPickMoveAccepted,
    kPickToggleSelectionAccepted,
    kPickDeselectAllAccepted,
};

class HUD {
public:
    HUD();
    ~HUD();
    static void Install();
    static void ShowPick(const RE::ObjectRefHandle& handle);
    static void ShowInventoryClone(
        const RE::ObjectRefHandle& handle,
        RE::TESBoundObject* item);
    static void ShowPlace();
    static void HidePick();
    static void HidePlace();
    static void OnMenuOpen();
    static void OnMenuClose();
    static void ProcessEvent(MenuEvent event, RE::NiPoint2 delta = {});
    static bool GetIsEnabled();
    static void SetIsEnabled(bool value);
    static void HideInventoryClone();

private:
    static inline bool isMenuOpen = false;
    static inline RE::ObjectRefHandle displayingRefHandle;
    static inline bool displayingPick = false;
    static inline bool displayingPlace = false;
};

