#pragma once
#include <glm/glm.hpp>

class Geometry {
    std::vector<RE::NiPoint3> positions;
    std::vector<uint16_t> indexes;

    void FetchVertexes(RE::BSGeometry* o3d, RE::BSGraphics::TriShape* triShape);
    void FetchBoneVertexes(RE::BSGeometry* o3d, RE::NiPointer<RE::NiSkinPartition>& partitions);
    void FetchIndexes(RE::BSGraphics::TriShape* triShape);

    RE::NiPoint3 Rotate(const RE::NiPoint3& A, const RE::NiPoint3& angles);

public:
    ~Geometry();
    Geometry(RE::NiAVObject* obj);
    bool Empty() const;
    std::pair<RE::NiPoint3, RE::NiPoint3> GetBoundingBox(RE::NiPoint3 position, RE::NiPoint3 angle, float scale);
    std::pair<RE::NiPoint3, RE::NiPoint3> GetWorldBoundingBox() const;
};
