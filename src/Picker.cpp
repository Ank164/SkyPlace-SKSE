#include "Picker.h"

#include <mutex>

#include "HUD.h"
#include "Placer.h"
#include "Raycast.h"
#include "Menu.h"
#include "ObjectGroup.h"
#include "Shader.h"


std::mutex picker_mutex;

RayOutput Cast() {
    return RayCast::Cast(
        [](RE::NiAVObject* obj) {
            if (obj) {
                if (obj->GetUserData()) {
                    const RE::ObjectRefHandle handle = obj->GetUserData()->GetHandle();
                    const RE::NiPointer<RE::TESObjectREFR> data = handle.get();
                    return !data || !data->IsPlayerRef();
                }
            }
            return true;
        },
        500);
}

RE::ObjectRefHandle Picker::GetLastHoverHandle() {
    if (!lastHoverHandle.get()) {
        lastHoverHandle.reset();
    }
    return lastHoverHandle;
}

void Picker::SaveChangeEvent() {
    HideSelectionHighlights();
    const RE::ObjectRefHandle hoverHandle = GetLastHoverHandle();
    if (hoverHandle) {
        Shader::ApplyPickableHighlight(hoverHandle);
    }
    lastHoverHandle.reset();
    selectedObjects.clear();
    HUD::HidePick();
}

void Picker::MoveEvent() {
    const RE::ObjectRefHandle hoverHandle = GetLastHoverHandle();

    if (hoverHandle) {
        HUD::HidePick();
        Shader::ClearReferenceHighlight(hoverHandle);
        lastHoverHandle.reset();
        Placer::Move(hoverHandle);
    }
}

void Picker::PickEvent() {
    std::vector<RE::ObjectRefHandle> targets = GetSelectedHandles();
    const RE::ObjectRefHandle hoverHandle = GetLastHoverHandle();

    if (hoverHandle && !IsSelected(hoverHandle)) {
        targets.push_back(hoverHandle);
    }

    DeselectAllEvent();
    if (hoverHandle) {
        Shader::ClearReferenceHighlight(hoverHandle);
    }
    lastHoverHandle.reset();

    PickObjects(targets);
}

bool Picker::IsSelected(const RE::ObjectRefHandle& handle) {
    const RE::NiPointer<RE::TESObjectREFR> refr = handle.get();
    if (!refr) {
        return false;
    }

    const RE::FormID formID = refr->GetFormID();
    for (const SelectedObject& selected : selectedObjects) {
        if (selected.formID == formID) {
            return true;
        }
    }

    return false;
}

bool Picker::IsLastHoverSelected() {
    return IsSelected(lastHoverHandle);
}

std::size_t Picker::GetSelectionCount() {
    return selectedObjects.size();
}

std::vector<RE::ObjectRefHandle> Picker::GetSelectedHandles() {
    std::vector<RE::ObjectRefHandle> handles;
    handles.reserve(selectedObjects.size());

    for (const SelectedObject& selected : selectedObjects) {
        handles.push_back(selected.handle);
    }

    return handles;
}

void Picker::SetSelection(const std::vector<RE::ObjectRefHandle>& handles) {
    HideSelectionHighlights();
    selectedObjects.clear();
    selectedObjects.reserve(handles.size());

    for (const RE::ObjectRefHandle& handle : handles) {
        const RE::NiPointer<RE::TESObjectREFR> selectedRef = handle.get();
        if (!selectedRef) {
            continue;
        }

        selectedObjects.push_back({selectedRef->GetFormID(), handle});
        Shader::ApplySelectedHighlight(handle);
    }
}

void Picker::RemoveReference(RE::FormID formID) {
    std::erase_if(selectedObjects, [formID](const SelectedObject& selected) {
        return selected.formID == formID;
    });
}

void Picker::ToggleSelectionEvent() {
    const RE::ObjectRefHandle hoverHandle = GetLastHoverHandle();
    const RE::NiPointer<RE::TESObjectREFR> lastHoverRef = hoverHandle.get();
    if (!lastHoverRef) {
        return;
    }

    const RE::FormID formID = lastHoverRef->GetFormID();
    for (auto selected = selectedObjects.begin(); selected != selectedObjects.end(); ++selected) {
        if (selected->formID == formID) {
            selectedObjects.erase(selected);
            Shader::ApplyHoverHighlight(hoverHandle);
            return;
        }
    }

    selectedObjects.push_back({formID, hoverHandle});
    Shader::ApplySelectedHighlight(hoverHandle);
}

void Picker::DeselectAllEvent() {
    HideSelectionHighlights();
    selectedObjects.clear();
    if (!Placer::IsPlacing()) {
        Shader::ApplyHoverHighlight(GetLastHoverHandle());
    }
}

void Picker::HideSelectionHighlights() {
    const RE::ObjectRefHandle hoverHandle = GetLastHoverHandle();
    for (const SelectedObject& selected : selectedObjects) {
        if (selected.handle == hoverHandle) {
            Shader::ApplyHoverHighlight(selected.handle);
        } else {
            Shader::ApplyPickableHighlight(selected.handle);
        }
    }
}

void Picker::ShowSelectionHighlights() {
    for (const SelectedObject& selected : selectedObjects) {
        Shader::ApplySelectedHighlight(selected.handle);
    }
}

void Picker::ShowPlacementHighlights(const RE::ObjectRefHandle& movingHandle) {
    ShowSelectionHighlights();
    if (movingHandle && !IsSelected(movingHandle)) {
        Shader::ApplyHoverHighlight(movingHandle);
    }
}

void Picker::HidePlacementHighlight(const RE::ObjectRefHandle& movingHandle) {
    if (!movingHandle) {
        return;
    }

    Shader::ClearReferenceHighlight(movingHandle);
    if (IsSelected(movingHandle)) {
        Shader::ApplySelectedHighlight(movingHandle);
    } else {
        Shader::ApplyPickableHighlight(movingHandle);
    }
}


void Picker::Tick() {

    if (Menu::IsOpen()) {
        return;
    }
    if (Placer::IsPlacing()) {
        return;
    }
    const RE::ObjectRefHandle previousHoverHandle = GetLastHoverHandle();
    RE::ObjectRefHandle nextHoverHandle;

    if (!Placer::IsPlacing() && HUD::GetIsEnabled()) {
        const auto result = Cast();
        if (result.hasHit && result.hitRef) {
            if (!previousHoverHandle || result.hitRef != previousHoverHandle) {
#ifndef NDEBUG
                const RE::NiPointer<RE::TESObjectREFR> hitRef = result.hitRef.get();
                if (hitRef) {
                    if (auto model = hitRef->GetBaseObject()->As<RE::TESModel>()) {
                        logger::trace("Model: {} ", model->GetModel());
                    }
                }
#endif  // !NDEBUG
            }

            if (Shader::IsMovable(result.hitRef)) {
                nextHoverHandle = result.hitRef;
            }
        }
    }

    if (lastHoverHandle != nextHoverHandle) {
        if (previousHoverHandle) {
            if (IsSelected(previousHoverHandle)) {
                Shader::ApplySelectedHighlight(previousHoverHandle);
            } else {
                Shader::ApplyPickableHighlight(previousHoverHandle);
            }
        }

        if (IsSelected(nextHoverHandle)) {
            Shader::ApplySelectedHighlight(nextHoverHandle);
        } else {
            Shader::ApplyHoverHighlight(nextHoverHandle);
        }

        if (nextHoverHandle) {
            HUD::ShowPick(nextHoverHandle);
        } else {
            HUD::HidePick();
        }
    }

    lastHoverHandle = nextHoverHandle;
}

bool Picker::PickObjects(const std::vector<RE::ObjectRefHandle>& handles) {
    std::lock_guard<std::mutex> lock(picker_mutex);
    return ObjectGroup::PickUp(handles);
}
