#include "SkyPlace.h"
#include "Picker.h"
#include "Placer.h"
#include "Transform.h"

#include <atomic>

namespace {
    std::atomic_bool pendingMove{false};
}

void PickUpObject(const RE::ObjectRefHandle& handle) {
    if (!handle) {
        return;
    }

    Picker::PickObjects({handle});
}

void PickUpMovingObject() { 
	Placer::PickEvent(); 
}

void PlaceMovingObject() { 
	Placer::PlaceEvent(); 
}

void CancelMovingObject() {
	Placer::CancelPlaceEvent();
}

void MoveObject(const RE::ObjectRefHandle& handle) {
	if (!handle || Placer::IsPlacing() || pendingMove.exchange(true)) {
		return;
	}

	const auto beginMove = [handle]() {
		pendingMove = false;
		Picker::MoveEvent(handle);
	};

	// External callers can invoke this API from inside another SkyPrompt
	// callback. Defer the HUD transition so SkyPlace does not try to replace a
	// prompt while SkyPrompt is still dispatching it.
	if (const SKSE::TaskInterface* tasks = SKSE::GetTaskInterface()) {
		tasks->AddTask(beginMove);
	} else {
		beginMove();
	}
}

bool IsMovingObject() {
	return pendingMove || Placer::IsPlacing();
}

bool SetObjectTransform(const RE::ObjectRefHandle& handle, const RE::NiPoint3& position,
                        const RE::NiPoint3& angle, float scale) {
	const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
	if (!ref) {
		return false;
	}

	Transform::SetPosition(handle, position);
	Transform::SetAngle(handle, angle);
	ref->SetScale(scale);
	if (ref->Is3DLoaded()) {
		ref->Update3DPosition(true);
	}
	return true;
}

void PlaceObjectFromPlayerInventory(RE::TESBoundObject* obj) { 
	Placer::RequestDrop(obj);
}
