#pragma once

namespace Input {
    enum Source {
        kNone,
        kKeyboard,
        kMouseButton,
        kGamepadOrbis,
        kGamepadDirectX,
        kThumbMove,
        kMouseMove,
        kTotal
    };
    using Device = RE::INPUT_DEVICE;
    using KeyboardKeys = RE::BSWin32KeyboardDevice::Key;
    using GamepadKeys = RE::BSWin32GamepadDevice::Key;
    using GamepadOrbisKeys = RE::BSPCOrbisGamepadDevice::Key;
    using MouseButtons = RE::BSWin32MouseDevice::Key;
    using Key = uint32_t;
    namespace {   
        enum Other {
            kMouseMove = 283,
            kThumbstickMove = 284
        };
    }
    Input::Source GetSource(RE::InputEvent* event);
}
