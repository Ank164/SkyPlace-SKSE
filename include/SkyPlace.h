#pragma once

#if IS_SKY_PLACE

#define FUNCTION_PREFIX extern "C" [[maybe_unused]] __declspec(dllexport)

FUNCTION_PREFIX void PickUpObject(const RE::ObjectRefHandle& handle);
FUNCTION_PREFIX void PickUpMovingObject();
FUNCTION_PREFIX void PlaceMovingObject();
FUNCTION_PREFIX void MoveObject(const RE::ObjectRefHandle& handle);
FUNCTION_PREFIX bool IsMovingObject();
FUNCTION_PREFIX void PlaceObjectFromPlayerInventory(RE::TESBoundObject* obj);

#else

#define API_FUNCTION_IMPL(name, functionName, result, parameters, callParameters)          \
result name(parameters) {                                                                  \
    using func_t = decltype(&name);                                                        \
    static auto menuFramework = GetModuleHandle(L"SkyPlace");                              \
    auto function = reinterpret_cast<func_t>(GetProcAddress(menuFramework, functionName)); \
    return function(callParameters);                                                       \
}


class SkyPlace {
public:
    static void PickUpObject(const RE::ObjectRefHandle& handle);
    static void PickUpMovingObject();
    static void PlaceMovingObject();
    static void MoveObject(const RE::ObjectRefHandle& handle);
    static bool IsMovingObject();
    static void PlaceObjectFromPlayerInventory(RE::TESBoundObject* obj);
};

API_FUNCTION_IMPL(SkyPlace::PickUpObject, "PickUpObject", void, const RE::ObjectRefHandle& handle, handle)
API_FUNCTION_IMPL(SkyPlace::PickUpMovingObject, "PickUpMovingObject", void, , )
API_FUNCTION_IMPL(SkyPlace::PlaceMovingObject, "PlaceMovingObject", void, , )
API_FUNCTION_IMPL(SkyPlace::MoveObject, "MoveObject", void, const RE::ObjectRefHandle& handle, handle)
API_FUNCTION_IMPL(SkyPlace::IsMovingObject, "IsMovingObject", bool, , )
API_FUNCTION_IMPL(SkyPlace::PlaceObjectFromPlayerInventory, "PlaceObjectFromPlayerInventory", void, RE::TESBoundObject* obj, obj)

#endif
