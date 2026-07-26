#pragma once

namespace ObjectGroup {
    struct Member {
        RE::FormID objectFormID = 0;
        RE::NiPoint3 relativePosition;
        RE::NiPoint3 relativeAngle;
        float scale = 1.0f;
        RE::NiPoint3 previewPosition;
        RE::NiMatrix3 previewRotation;
        float previewScale = 1.0f;
        RE::FormID inventoryChestRefID = 0;
    };

    struct Data {
        RE::FormID itemFormID = 0;
        std::string guid;
        std::string meshPath;
        std::string name;
        float weight = 0.0f;
        std::int32_t value = 0;
        float playerFacingYaw = 0.0f;
        bool preservesPlayerFacing = false;
        std::vector<Member> members;
    };

    void Clear();
    bool PickUp(const std::vector<RE::ObjectRefHandle>& handles);
    bool IsGroupItem(RE::TESBoundObject* item);
    bool Materialize(
        RE::TESBoundObject* item,
        std::vector<RE::ObjectRefHandle>& materializedHandles);
    void RetireIfUnused(RE::TESBoundObject* item);
    void ReviveItem(RE::TESObjectMISC* item, RE::FormID savedFormID);
    const Data* GetByMesh(std::string_view meshPath);
    const std::map<RE::FormID, Data>& GetAll();
    const std::vector<RE::FormID>& GetRetiredItems();
    void Restore(Data data);
}
