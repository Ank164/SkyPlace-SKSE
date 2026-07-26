#pragma once

using InputEventCallback = std::function<bool(RE::InputEvent*)>;
//using InputEventCallback = bool(*)(RE::InputEvent*);

class InputEventHandler {
    static inline std::vector<InputEventCallback> callbacks;
    static inline bool middleButtonDown = false;
    static bool InputEvent(RE::InputEvent* event);
public:
    static RE::InputEvent* const* Process(RE::InputEvent* const* a_event);
    static void Register(InputEventCallback callback);
};
