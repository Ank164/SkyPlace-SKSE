#include "Input.h"




Input::Source Input::GetSource(RE::InputEvent* event) {
    if (event->AsMouseMoveEvent()) {
        return Source::kMouseMove;
    }
    if (event->AsThumbstickEvent()) {
        return Source::kThumbMove;
    }

    switch (event->GetDevice()) {

        case RE::INPUT_DEVICES::kKeyboard:
            return Source::kKeyboard;
        case RE::INPUT_DEVICES::kMouse:
            return Source::kMouseButton;
        case RE::INPUT_DEVICES::kGamepad:
            return Source::kGamepadDirectX;
        default:
            return Source::kNone;
    }
}
