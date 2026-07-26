#pragma once

namespace InventoryChest {
    bool ShouldStore(const RE::ObjectRefHandle& handle);
    RE::ObjectRefHandle GetStorageMarker();
    RE::FormID Store(const RE::ObjectRefHandle& sourceHandle);
    void Restore(RE::FormID chestRefID, const RE::ObjectRefHandle& targetHandle);
}
