#include "SkyPlace.h"
#include "Picker.h"
#include "Placer.h"

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

void MoveObject(const RE::ObjectRefHandle& handle) {
	Placer::Move(handle);
}

void PlaceObjectFromPlayerInventory(RE::TESBoundObject* obj) { 
	Placer::RequestDrop(obj);
}
