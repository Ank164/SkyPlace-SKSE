#include "HUD.h"
#include "Hooks.h"
#include "Picker.h"
#include "Placer.h"
#include "Shader.h"
#include "SkyPromptClient.h"

HUD::HUD() {}

HUD::~HUD() {}

void HUD::Install() {
	SkyPromptClient::Install(); 
}

void HUD::ShowPick(const RE::ObjectRefHandle& handle) {
    const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
    if (!ref) {
        HidePick();
        return;
    }

    if (displayingPick && displayingRefHandle == handle) {
        return;
    }
    displayingRefHandle = handle;
    displayingPick = true;
    if (!isMenuOpen) {
        SkyPromptClient::ShowPick(handle);
    }
}

void HUD::ShowInventoryClone(
    const RE::ObjectRefHandle& handle,
    RE::TESBoundObject* item) {
    const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
    if (!ref || !item) {
        HideInventoryClone();
        return;
    }

    displayingRefHandle = handle;
    if (isMenuOpen) {
        SkyPromptClient::ShowInventoryClone(handle, item);
    }
}

void HUD::HideInventoryClone() {
    SkyPromptClient::HideInventoryClone();
}

void HUD::ShowPlace() {
    if (displayingPlace) {
        return;
    }
    displayingPlace = true;
    if (!isMenuOpen) {
        SkyPromptClient::ShowPlace();
    }
}

void HUD::HidePick() {
    displayingRefHandle.reset();
    displayingPick = false;
    SkyPromptClient::HidePick();
}

void HUD::HidePlace() {
    displayingPlace = false;
    SkyPromptClient::HidePlace();
}

void HUD::OnMenuOpen() {
    if (!isMenuOpen) {
        isMenuOpen = true;
        SkyPromptClient::HidePick(); 
        SkyPromptClient::HidePlace();
    }
}

void HUD::OnMenuClose() {
    if (isMenuOpen) {
        isMenuOpen = false;
        if (displayingPlace) {
            SkyPromptClient::ShowPlace();
        }
        else if (displayingPick) {
            if (displayingRefHandle.get()) {
                SkyPromptClient::ShowPick(displayingRefHandle);
            } else {
                displayingRefHandle.reset();
                displayingPick = false;
            }
        }
        SkyPromptClient::HideInventoryClone();
    }
}

void HUD::ProcessEvent(MenuEvent event, RE::NiPoint2 delta) {
    switch (event) {
        case kPlacerClose:
            Placer::CancelPlaceEvent();
            break;
        case kPlacePickAccepted:
            Placer::PickEvent();
            break;
        case kPlacePlaceAccepted:
            Placer::PlaceEvent();
            break;
        case kPlaceTranslateUpDown:
            Placer::TranslateUpDownEvent(delta);
            break;
        case kPlaceTranslateLeftRight:
            Placer::TranslateLeftRightEvent(delta);
            break;
        case kPlaceTranslateForwardBackward:
            Placer::TranslateForwardBackwardEvent(delta);
            break;
        case kPlaceTranslateDepth:
            Placer::TranslateDepthEvent(delta);
            break;
        case kPlaceSetRaycastDistance:
            Placer::SetRaycastDistanceFromCurrentPosition();
            break;
        case kPlaceOrbitRotate:
            Placer::OrbitRotateEvent(delta);
            break;
        case kPlaceScale:
            Placer::ScaleEvent(delta);
            break;

        case kPickPickAccepted:
            Picker::PickEvent();
            break;
        case kPickMoveAccepted:
            Picker::MoveEvent();
            break;
        case kPickToggleSelectionAccepted:
            Picker::ToggleSelectionEvent();
            break;
        case kPickDeselectAllAccepted:
            Picker::DeselectAllEvent();
            break;
        default:
            break;
    }
}
bool HUD::GetIsEnabled() {
    return SkyPromptClient::GetIsEnabled();
}

void HUD::SetIsEnabled(bool value) {
    const bool wasEnabled = SkyPromptClient::GetIsEnabled();
    SkyPromptClient::SetIsEnabled(value);
    if (!value) {
        Picker::SaveChangeEvent();
        Shader::QueueClearAllReferenceHighlights();
    } else if (!wasEnabled) {
        Hooks::RefreshLoadedReferenceHighlights();
    }
}
