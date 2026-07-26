#include "Raycast.h"
#include "DrawDebug.h"


enum class LineOfSightLocation : uint32_t { kNone, kEyes, kHead, kTorso, kFeet };

namespace {
    RE::ObjectRefHandle GetPickedReference(RE::NiAVObject* object) {
        while (object) {
            if (object->GetUserData()) {
                return object->GetUserData()->GetHandle();
            }
            object = object->parent;
        }
        return {};
    }

    RayOutput CastConsolePick(
        const RE::NiPoint3& position,
        const RE::NiPoint3& direction,
        const std::function<bool(RE::NiAVObject*)>& evaluator,
        const float raySize) {
        RE::SceneGraph* worldRoot = RE::Main::WorldRootNode();
        if (!worldRoot) {
            return {};
        }

        RE::NiPick::Ptr picker = RE::NiPick::Create(16, 16);
        if (!picker) {
            return {};
        }

        picker->root.reset(worldRoot);
        picker->pickType = RE::NiPick::PickType::FIND_ALL;
        picker->sortType = RE::NiPick::SortType::SORT;
        picker->intersectType = RE::NiPick::IntersectType::TRIANGLE_INTERSECT;
        picker->coordinateType = RE::NiPick::CoordinateType::WORLD_COORDINATES;
        picker->frontOnly = false;
        picker->observeAppCullFlag = true;
        picker->returnNormal = true;

        if (!picker->PickObjects(position, direction)) {
            return {};
        }

        const std::uint16_t resultCount = picker->pickResults.free_idx();
        for (std::uint16_t index = 0; index < resultCount; ++index) {
            RE::NiPick::Record* record = picker->pickResults[index];
            if (!record || record->distance < 0.0f || record->distance > raySize) {
                continue;
            }

            RE::NiAVObject* pickedObject = record->object.get();
            const RE::ObjectRefHandle referenceHandle = GetPickedReference(pickedObject);
            const RE::NiPointer<RE::TESObjectREFR> reference = referenceHandle.get();
            RE::NiAVObject* reference3D = reference ? reference->Get3D() : nullptr;
            if (!reference3D || !evaluator(reference3D)) {
                continue;
            }

            return RayOutput{
                record->normal,
                record->intersect,
                raySize > 0.0f ? record->distance / raySize : 0.0f,
                nullptr,
                referenceHandle,
                true};
        }

        return {};
    }
}

RE::NiPoint3 RayCast::QuaternionToEuler(const RE::NiQuaternion& q) {
    RE::NiPoint3 euler;

    const double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    const double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    euler.x = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    if (const double sinp = 2 * (q.w * q.y - q.z * q.x); std::abs(sinp) >= 1)
        euler.y = std::copysign(glm::pi<float>() / 2, sinp);
    else
        euler.y = std::asin(sinp);

    // Yaw (z-axis rotation)
    const double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    const double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    euler.z = std::atan2(siny_cosp, cosy_cosp);

    euler.x = euler.x * -1;
    //euler.y = euler.y;
    euler.z = euler.z * -1;

    return euler;
}

bool RayCast::IsGameplayCameraActive() {
    const RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();
    if (!camera) {
        return false;
    }

    RE::BSSpinLockGuard spin(camera->GetRuntimeData().lock);
    if (!camera->currentState) {
        return false;
    }

    return camera->currentState->id == RE::CameraState::kFirstPerson ||
        camera->currentState->id == RE::CameraState::kThirdPerson;
}


std::pair<RE::NiPoint3, RE::NiPoint3> RayCast::GetCameraData() {
    const RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();

    RE::BSSpinLockGuard spin(camera->GetRuntimeData().lock);

    const auto thirdPerson = reinterpret_cast<RE::ThirdPersonState*>(camera->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
    const auto firstPerson = reinterpret_cast<RE::FirstPersonState*>(camera->GetRuntimeData().cameraStates[RE::CameraState::kFirstPerson].get());

    RE::NiQuaternion rotation;
    RE::NiPoint3 translation;
    if (camera->currentState->id == RE::CameraState::kFirstPerson) {
        firstPerson->GetRotation(rotation);
        firstPerson->GetTranslation(translation);
        translation += firstPerson->dampeningOffset;
    } else if (camera->currentState->id == RE::CameraState::kThirdPerson) {
        rotation = thirdPerson->rotation;
        translation = thirdPerson->translation;
    } else {
        return {};
    }
    return {QuaternionToEuler(rotation), translation};
}

RayOutput RayCast::Cast(std::function<bool(RE::NiAVObject*)> const& evaluator, const float raySize) {

    if (!IsGameplayCameraActive()) {
        return {};
    }

    auto [camera_rotation, camera_position] = GetCameraData();

    const RayOutput consolePick =
        CastConsolePick(camera_position, RayMath::angles2dir(camera_rotation), evaluator, raySize);
    const RayOutput physicsPick = CastRay(camera_rotation, camera_position, evaluator, raySize);

    if (!consolePick.hasHit) {
        return physicsPick;
    }

    if (physicsPick.hasHit && !physicsPick.hitRef && physicsPick.hitFraction < consolePick.hitFraction) {
        return physicsPick;
    }

    return consolePick;
}

RayOutput RayCast::CastRay(
    RE::NiPoint3 angle, RE::NiPoint3 position,
    std::function<bool(RE::NiAVObject*)> const& evaluator, float raySize) {
    using namespace RayMath;
    auto havokWorldScale = RE::bhkWorld::GetWorldScale();
    RE::bhkPickData pick_data;
    RE::NiPoint3 ray_start, ray_end;

    ray_start = position;
    ray_end = ray_start + rotate(raySize, angle);
    pick_data.rayInput.from = ray_start * havokWorldScale;
    pick_data.rayInput.to = ray_end * havokWorldScale;

    //DrawDebug::DrawLine(ray_start, ray_end, {1,0,0,1});

    auto dif = ray_start - ray_end;

    auto collector = RayCollector(evaluator);
    collector.Reset();
    pick_data.closestRayHitCollector = reinterpret_cast<RE::hkpClosestRayHitCollector*>(&collector);

    const auto ply = RE::PlayerCharacter::GetSingleton();
    if (!ply->parentCell) return {};

    if (auto physicsWorld = ply->parentCell->GetbhkWorld()) {
        physicsWorld->PickObject(pick_data);
    }

    RayCollector::HitResult best = {};
    best.hitFraction = 1.0f;
    RE::NiPoint3 bestPos = {};

    for (auto& hit : collector.GetHits()) {
        const auto pos = (dif * hit.hitFraction) + ray_start;
        if (best.body == nullptr) {
            best = hit;
            bestPos = pos;
            continue;
        }

        if (hit.hitFraction < best.hitFraction) {
            best = hit;
            bestPos = pos;
        }
    }


    if (!best.body) {
        return RayOutput{RE::NiPoint3{best.normal.x, best.normal.y, best.normal.z}, ray_end, best.hitFraction,
                         best.body, {}, false};
    }

    auto hitpos = ray_start + (ray_end - ray_start) * best.hitFraction;

    if (auto av = best.getAVObject()) {
        const RE::ObjectRefHandle handle =
            av->GetUserData() ? av->GetUserData()->GetHandle() : RE::ObjectRefHandle{};

        return RayOutput{RE::NiPoint3{best.normal.x, best.normal.y, best.normal.z}, hitpos, best.hitFraction,
                         best.body, handle, true};

    }

    return RayOutput{RE::NiPoint3{best.normal.x, best.normal.y, best.normal.z}, hitpos, best.hitFraction, best.body, {}, true};
}
