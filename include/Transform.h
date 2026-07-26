#pragma once
class Transform {
    static inline void MoveTo_Impl(
        const RE::ObjectRefHandle& handle,
        const RE::ObjectRefHandle& a_targetHandle,
        RE::TESObjectCELL* a_targetCell,
        RE::TESWorldSpace* a_selfWorldSpace,
        const RE::NiPoint3& a_position,
        const RE::NiPoint3& a_rotation) {
        using func_t = void(
            RE::TESObjectREFR*,
            const RE::ObjectRefHandle&,
            RE::TESObjectCELL*,
            RE::TESWorldSpace*,
            const RE::NiPoint3&,
            const RE::NiPoint3&);
        REL::Relocation<func_t> func{RELOCATION_ID(56227, 56626)};
        const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
        if (!ref) {
            return;
        }
        return func(
            ref.get(),
            a_targetHandle,
            a_targetCell,
            a_selfWorldSpace,
            a_position,
            a_rotation);
    }

public:
    static inline void ClearFade(const RE::ObjectRefHandle& handle) {
        const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
        if (!ref) {
            return;
        }

        RE::NiAVObject* object3D = ref->Get3D();
        if (!object3D) {
            return;
        }

        RE::BSFadeNode* fade = object3D->AsFadeNode();
        if (fade) {
            fade->GetRuntimeData().currentFade = 1.f;
        }
    }
    static inline void Wrap(
        const RE::ObjectRefHandle& handle,
        RE::NiPoint3 pos,
        RE::NiPoint3 angle) {
        const RE::NiPointer<RE::TESObjectREFR> obj = handle.get();
        if (!obj) {
            return;
        }

        if (obj->GetBaseObject()->IsInventoryObject()) {
            SetPosition(handle, pos);
            SetAngle(handle, angle);
            if (obj->Is3DLoaded()) {
                obj->Update3DPosition(true);
            }
            return;
        }
        MoveTo_Impl(handle, handle, obj->GetParentCell(), obj->GetWorldspace(), pos, angle);
        ClearFade(handle);

    }
    static inline void SetPosition(
        const RE::ObjectRefHandle& handle,
        const RE::NiPoint3& a_position) {
        const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
        if (!ref) {
            return;
        }
        using func_t = void(RE::TESObjectREFR*, const RE::NiPoint3&);
        const REL::Relocation<func_t> func{RELOCATION_ID(19363, 19790)};
        return func(ref.get(), a_position);
    }

    static inline void SetAngle(
        const RE::ObjectRefHandle& handle,
        const RE::NiPoint3& a_position) {
        const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
        if (!ref) {
            return;
        }
        using func_t = void(RE::TESObjectREFR*, const RE::NiPoint3&);
        const REL::Relocation<func_t> func{RELOCATION_ID(19359, 19786)};
        return func(ref.get(), a_position);
    }
};
