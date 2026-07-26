#include "InventoryChest.h"

#include "Inventory.h"

namespace {
    constexpr RefID unownedChestOGRefID = 0x000EA29A;
    constexpr RE::FormID emptyChestBaseForm = 0xF3923;

    std::vector<std::pair<RE::TESBoundObject*, std::int32_t>> GetInventoryItems(
        const RE::ObjectRefHandle& handle) {
        std::vector<std::pair<RE::TESBoundObject*, std::int32_t>> items;
        const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
        if (!ref || !ref->HasContainer()) {
            return items;
        }

        for (const auto& [item, invData] : ref->GetInventory()) {
            const auto& [count, entry] = invData;
            if (entry && entry->object && count > 0) {
                items.push_back({entry->object, count});
            } else if (item && count > 0) {
                items.push_back({item, count});
            }
        }

        return items;
    }

    RE::TESBoundObject* GetChestBase() {
        return RE::TESForm::LookupByID<RE::TESBoundObject>(emptyChestBaseForm);
    }

    RE::ObjectRefHandle PlaceStorageChest() {
        const RE::ObjectRefHandle markerHandle = InventoryChest::GetStorageMarker();
        const RE::NiPointer<RE::TESObjectREFR> marker = markerHandle.get();
        RE::TESBoundObject* chestBase = GetChestBase();
        if (!marker || !chestBase) {
            return {};
        }

        RE::NiPointer<RE::TESObjectREFR> chest = marker->PlaceObjectAtMe(chestBase, true);
        if (!chest) {
            return {};
        }

        chest->AddChange(
            RE::TESObjectREFR::ChangeFlags::kCellChanged |
            RE::TESObjectREFR::ChangeFlags::kMoved |
            RE::TESObjectREFR::ChangeFlags::kBaseObject);
        return chest->GetHandle();
    }

    void TransferInventory(
        const RE::ObjectRefHandle& sourceHandle,
        const RE::ObjectRefHandle& targetHandle) {
        const RE::NiPointer<RE::TESObjectREFR> source = sourceHandle.get();
        const RE::NiPointer<RE::TESObjectREFR> target = targetHandle.get();
        if (!source || !target) {
            return;
        }

        const std::vector<std::pair<RE::TESBoundObject*, std::int32_t>> items = GetInventoryItems(sourceHandle);
        for (const auto& [item, count] : items) {
            source->RemoveItem(
                item,
                count,
                RE::ITEM_REMOVE_REASON::kRemove,
                nullptr,
                target.get());
        }
    }

    void QueueRemoval(RE::ObjectRefHandle handle) {
        const SKSE::TaskInterface* taskInterface = SKSE::GetTaskInterface();
        if (!taskInterface) {
            return;
        }

        taskInterface->AddTask([handle]() {
            const RE::NiPointer<RE::TESObjectREFR> chest = handle.get();
            if (chest) {
                RE::GarbageCollector::GetSingleton()->Add(chest.get(), true);
            }
        });
    }
}

bool InventoryChest::ShouldStore(const RE::ObjectRefHandle& handle) {
    const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
    return ref && ref->HasContainer();
}

RE::ObjectRefHandle InventoryChest::GetStorageMarker() {
    const RE::NiPointer<RE::TESObjectREFR> marker{
        RE::TESForm::LookupByID<RE::TESObjectREFR>(unownedChestOGRefID)};
    return marker ? marker->GetHandle() : RE::ObjectRefHandle{};
}

RE::FormID InventoryChest::Store(const RE::ObjectRefHandle& sourceHandle) {
    if (!ShouldStore(sourceHandle)) {
        return 0;
    }

    const RE::ObjectRefHandle chestHandle = PlaceStorageChest();
    const RE::NiPointer<RE::TESObjectREFR> chest = chestHandle.get();
    if (!chest) {
        return 0;
    }

    TransferInventory(sourceHandle, chestHandle);
    return chest->GetFormID();
}

void InventoryChest::Restore(
    RE::FormID chestRefID,
    const RE::ObjectRefHandle& targetHandle) {
    if (!targetHandle) {
        return;
    }

    Inventory::Remove(targetHandle, RE::ITEM_REMOVE_REASON::kRemove);
    if (chestRefID == 0) {
        return;
    }

    const RE::NiPointer<RE::TESObjectREFR> chest{
        RE::TESForm::LookupByID<RE::TESObjectREFR>(chestRefID)};
    if (!chest) {
        return;
    }
    const RE::ObjectRefHandle chestHandle = chest->GetHandle();

    TransferInventory(chestHandle, targetHandle);
    QueueRemoval(chestHandle);
}
