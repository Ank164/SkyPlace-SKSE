#pragma once
#include "Controls.h"
#include <shared_mutex>
class Placer {
    struct GroupMember {
        RE::ObjectRefHandle handle;
        RE::NiPoint3 initialPosition;
        RE::NiPoint3 initialAngle;
        RE::NiPoint3 currentPosition;
        RE::NiPoint3 currentAngle;
        bool hasPlacementHighlight;
    };

    static inline RE::NiPoint3 translation;
    static inline RE::NiPoint3 initialPosition;
    static inline RE::NiPoint3 initialAngle;
    static inline RE::NiPoint3 currentPosition;
    static inline RE::NiPoint3 currentRaycastPosition;
    static inline RE::NiPoint3 currentAngle;
    static inline RE::NiMatrix3 initialOrientation;
    static inline RE::NiMatrix3 currentOrientation;
    static inline float initialCameraYaw = 0.0f;
    static inline float appliedHorizontalAngle = 0.0f;
    static inline float raycastDistance = 500.0f;
    static inline RE::ObjectRefHandle moveHandle;
    static inline bool inventorySource = false;
    static RE::NiPoint3 Cast();
    static void Rotate();
    static void ClearHints();
    static inline Controls* controls;
    static inline RE::FormID roomFormId;
    static inline unsigned long numNormals = 0; 
    static void PreventFloorClipping();
    static inline std::shared_mutex mtx;
    static RE::ObjectRefHandle GetMoveHandle();
    static void UpdateObjectRoom(const RE::ObjectRefHandle& handle);
    static inline std::vector<GroupMember> groupMembers;
    static void BeginGroupMove();
    static void ApplyGroupTransform();
    static void ShowGroupPlacementHighlights();
    static void FinishGroupMove(bool restoreOriginalTransform);
    static void ProcessPendingDrop();
    static void ProcessPendingMaterializedMove();
    static inline RE::FormID pendingDropFormID = 0;
    static inline bool pendingDropItemRemoved = false;
    static inline std::uint8_t pendingDropSafeTicks = 0;
    static inline std::vector<RE::ObjectRefHandle> pendingMaterializedHandles;
    static inline std::uint8_t pendingMaterializedSafeTicks = 0;

public:
    static bool IsGroupMember(const RE::ObjectRefHandle& handle);
    static void SaveChangeEvent();
    static void Tick();
    static void Move(const RE::ObjectRefHandle& handle);
    static bool RequestDrop(RE::TESBoundObject* refr, bool itemRemoved = false);
    static bool Drop(RE::TESBoundObject* refr, bool itemRemoved = false);
    static bool IsPlacing();
    static void Translate(RE::NiPoint3 offset);

    static void PlaceEvent();
    static void TranslateUpDownEvent(RE::NiPoint2 delta);
    static void TranslateLeftRightEvent(RE::NiPoint2 delta);
    static void TranslateDepthEvent(RE::NiPoint2 delta);
    static void SetRaycastDistanceFromCurrentPosition();
    static void OrbitRotateEvent(RE::NiPoint2 delta);
    static void PickEvent();
    static void CancelPlaceEvent();
};
