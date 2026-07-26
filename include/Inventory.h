#pragma once
class Inventory {
public:
    static inline void Remove(
        const RE::ObjectRefHandle& handle,
        RE::ITEM_REMOVE_REASON reason = RE::ITEM_REMOVE_REASON::kDropping) {
        const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
        if (!ref) {
            return;
        }
        if (ref->HasContainer()) {
            std::map<RE::TESBoundObject*, int> drop;

            for (const auto& [item, invData] : ref->GetInventory()) {
                const auto& [count, entry] = invData;
                RE::TESBoundObject* object =
                    entry && entry->object ? entry->object : item;
                if (object && count > 0) {
                    drop[object] = count;
                }
            }

            for (const auto& [obj, count] : drop) {
                ref->RemoveItem(obj, count, reason, nullptr, nullptr);
            }
        }
    }
};
