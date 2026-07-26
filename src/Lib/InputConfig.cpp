#include "InputConfig.h"
#include "Translations.h"

#include <chrono>
#include <tuple>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace InputConfig {
    constexpr const char* inputConfigFolder = "Data/SKSE/Plugins/SkyPlaceKeyBindings.json";

    enum class Trigger {
        kPress,
        kDoubleTap
    };

    struct Binding {
        RE::INPUT_DEVICE device;
        uint32_t key;
        Trigger trigger;
    };

    using TapClock = std::chrono::steady_clock;
    using TapKey = std::tuple<std::string, RE::INPUT_DEVICE, uint32_t>;

    constexpr std::chrono::milliseconds doubleTapInterval{350};
    static inline std::map<std::string, std::vector<Binding>> input_configs;
    static inline std::map<TapKey, TapClock::time_point> lastTapTimes;
}

inline std::map<std::string, RE::INPUT_DEVICE> deviceMap = {
    {"keyboard", RE::INPUT_DEVICE::kKeyboard},
    {"mouse", RE::INPUT_DEVICE::kMouse},
    {"gamepad", RE::INPUT_DEVICE::kGamepad},
};

inline std::map<std::string, RE::BSKeyboardDevice::Key> keyboardMap = {
    {"escape", RE::BSKeyboardDevice::Key::kEscape},
    {"num1", RE::BSKeyboardDevice::Key::kNum1},
    {"num2", RE::BSKeyboardDevice::Key::kNum2},
    {"num3", RE::BSKeyboardDevice::Key::kNum3},
    {"num4", RE::BSKeyboardDevice::Key::kNum4},
    {"num5", RE::BSKeyboardDevice::Key::kNum5},
    {"num6", RE::BSKeyboardDevice::Key::kNum6},
    {"num7", RE::BSKeyboardDevice::Key::kNum7},
    {"num8", RE::BSKeyboardDevice::Key::kNum8},
    {"num9", RE::BSKeyboardDevice::Key::kNum9},
    {"num0", RE::BSKeyboardDevice::Key::kNum0},
    {"minus", RE::BSKeyboardDevice::Key::kMinus},
    {"equals", RE::BSKeyboardDevice::Key::kEquals},
    {"backspace", RE::BSKeyboardDevice::Key::kBackspace},
    {"tab", RE::BSKeyboardDevice::Key::kTab},
    {"q", RE::BSKeyboardDevice::Key::kQ},
    {"w", RE::BSKeyboardDevice::Key::kW},
    {"e", RE::BSKeyboardDevice::Key::kE},
    {"r", RE::BSKeyboardDevice::Key::kR},
    {"t", RE::BSKeyboardDevice::Key::kT},
    {"y", RE::BSKeyboardDevice::Key::kY},
    {"u", RE::BSKeyboardDevice::Key::kU},
    {"i", RE::BSKeyboardDevice::Key::kI},
    {"o", RE::BSKeyboardDevice::Key::kO},
    {"p", RE::BSKeyboardDevice::Key::kP},
    {"bracketleft", RE::BSKeyboardDevice::Key::kBracketLeft},
    {"bracketright", RE::BSKeyboardDevice::Key::kBracketRight},
    {"enter", RE::BSKeyboardDevice::Key::kEnter},
    {"leftcontrol", RE::BSKeyboardDevice::Key::kLeftControl},
    {"a", RE::BSKeyboardDevice::Key::kA},
    {"s", RE::BSKeyboardDevice::Key::kS},
    {"d", RE::BSKeyboardDevice::Key::kD},
    {"f", RE::BSKeyboardDevice::Key::kF},
    {"g", RE::BSKeyboardDevice::Key::kG},
    {"h", RE::BSKeyboardDevice::Key::kH},
    {"j", RE::BSKeyboardDevice::Key::kJ},
    {"k", RE::BSKeyboardDevice::Key::kK},
    {"l", RE::BSKeyboardDevice::Key::kL},
    {"semicolon", RE::BSKeyboardDevice::Key::kSemicolon},
    {"apostrophe", RE::BSKeyboardDevice::Key::kApostrophe},
    {"tilde", RE::BSKeyboardDevice::Key::kTilde},
    {"leftshift", RE::BSKeyboardDevice::Key::kLeftShift},
    {"backslash", RE::BSKeyboardDevice::Key::kBackslash},
    {"z", RE::BSKeyboardDevice::Key::kZ},
    {"x", RE::BSKeyboardDevice::Key::kX},
    {"c", RE::BSKeyboardDevice::Key::kC},
    {"v", RE::BSKeyboardDevice::Key::kV},
    {"b", RE::BSKeyboardDevice::Key::kB},
    {"n", RE::BSKeyboardDevice::Key::kN},
    {"m", RE::BSKeyboardDevice::Key::kM},
    {"comma", RE::BSKeyboardDevice::Key::kComma},
    {"period", RE::BSKeyboardDevice::Key::kPeriod},
    {"slash", RE::BSKeyboardDevice::Key::kSlash},
    {"rightshift", RE::BSKeyboardDevice::Key::kRightShift},
    {"kp_multiply", RE::BSKeyboardDevice::Key::kKP_Multiply},
    {"leftalt", RE::BSKeyboardDevice::Key::kLeftAlt},
    {"spacebar", RE::BSKeyboardDevice::Key::kSpacebar},
    {"capslock", RE::BSKeyboardDevice::Key::kCapsLock},
    {"f1", RE::BSKeyboardDevice::Key::kF1},
    {"f2", RE::BSKeyboardDevice::Key::kF2},
    {"f3", RE::BSKeyboardDevice::Key::kF3},
    {"f4", RE::BSKeyboardDevice::Key::kF4},
    {"f5", RE::BSKeyboardDevice::Key::kF5},
    {"f6", RE::BSKeyboardDevice::Key::kF6},
    {"f7", RE::BSKeyboardDevice::Key::kF7},
    {"f8", RE::BSKeyboardDevice::Key::kF8},
    {"f9", RE::BSKeyboardDevice::Key::kF9},
    {"f10", RE::BSKeyboardDevice::Key::kF10},
    {"numlock", RE::BSKeyboardDevice::Key::kNumLock},
    {"scrolllock", RE::BSKeyboardDevice::Key::kScrollLock},
    {"kp_7", RE::BSKeyboardDevice::Key::kKP_7},
    {"kp_8", RE::BSKeyboardDevice::Key::kKP_8},
    {"kp_9", RE::BSKeyboardDevice::Key::kKP_9},
    {"kp_subtract", RE::BSKeyboardDevice::Key::kKP_Subtract},
    {"kp_4", RE::BSKeyboardDevice::Key::kKP_4},
    {"kp_5", RE::BSKeyboardDevice::Key::kKP_5},
    {"kp_6", RE::BSKeyboardDevice::Key::kKP_6},
    {"kp_plus", RE::BSKeyboardDevice::Key::kKP_Plus},
    {"kp_1", RE::BSKeyboardDevice::Key::kKP_1},
    {"kp_2", RE::BSKeyboardDevice::Key::kKP_2},
    {"kp_3", RE::BSKeyboardDevice::Key::kKP_3},
    {"kp_0", RE::BSKeyboardDevice::Key::kKP_0},
    {"kp_decimal", RE::BSKeyboardDevice::Key::kKP_Decimal},

    {"f11", RE::BSKeyboardDevice::Key::kF11},
    {"f12", RE::BSKeyboardDevice::Key::kF12},

    {"kp_enter", RE::BSKeyboardDevice::Key::kKP_Enter},
    {"rightcontrol", RE::BSKeyboardDevice::Key::kRightControl},

    {"kp_divide", RE::BSKeyboardDevice::Key::kKP_Divide},

    {"printscreen", RE::BSKeyboardDevice::Key::kPrintScreen},
    {"rightalt", RE::BSKeyboardDevice::Key::kRightAlt},

    {"pause", RE::BSKeyboardDevice::Key::kPause},

    {"home", RE::BSKeyboardDevice::Key::kHome},
    {"up", RE::BSKeyboardDevice::Key::kUp},
    {"pageup", RE::BSKeyboardDevice::Key::kPageUp},

    {"left", RE::BSKeyboardDevice::Key::kLeft},

    {"right", RE::BSKeyboardDevice::Key::kRight},

    {"end", RE::BSKeyboardDevice::Key::kEnd},
    {"down", RE::BSKeyboardDevice::Key::kDown},
    {"pagedown", RE::BSKeyboardDevice::Key::kPageDown},
    {"insert", RE::BSKeyboardDevice::Key::kInsert},
    {"delete", RE::BSKeyboardDevice::Key::kDelete},

    {"leftwin", RE::BSKeyboardDevice::Key::kLeftWin},
    {"rightwin", RE::BSKeyboardDevice::Key::kRightWin}}; 

inline std::map<std::string, RE::BSWin32MouseDevice::Key> mouseMap = {
    {"leftbutton", RE::BSWin32MouseDevice::Key::kLeftButton},
    {"rightbutton", RE::BSWin32MouseDevice::Key::kRightButton},
    {"middlebutton", RE::BSWin32MouseDevice::Key::kMiddleButton},
    {"button3", RE::BSWin32MouseDevice::Key::kButton3},
    {"button4", RE::BSWin32MouseDevice::Key::kButton4},
    {"button5", RE::BSWin32MouseDevice::Key::kButton5},
    {"button6", RE::BSWin32MouseDevice::Key::kButton6},
    {"button7", RE::BSWin32MouseDevice::Key::kButton7},
    {"wheelup", RE::BSWin32MouseDevice::Key::kWheelUp},
    {"wheeldown", RE::BSWin32MouseDevice::Key::kWheelDown}}; 

inline std::map<std::string, RE::BSWin32GamepadDevice::Key> gamepadMap = {
    {"up", RE::BSWin32GamepadDevice::Key::kUp},
    {"down", RE::BSWin32GamepadDevice::Key::kDown},
    {"left", RE::BSWin32GamepadDevice::Key::kLeft},
    {"right", RE::BSWin32GamepadDevice::Key::kRight},
    {"start", RE::BSWin32GamepadDevice::Key::kStart},
    {"back", RE::BSWin32GamepadDevice::Key::kBack},
    {"leftthumb", RE::BSWin32GamepadDevice::Key::kLeftThumb},
    {"rightthumb", RE::BSWin32GamepadDevice::Key::kRightThumb},
    {"leftshoulder", RE::BSWin32GamepadDevice::Key::kLeftShoulder},
    {"rightshoulder", RE::BSWin32GamepadDevice::Key::kRightShoulder},
    {"a", RE::BSWin32GamepadDevice::Key::kA},
    {"b", RE::BSWin32GamepadDevice::Key::kB},
    {"x", RE::BSWin32GamepadDevice::Key::kX},
    {"y", RE::BSWin32GamepadDevice::Key::kY},
    {"lefttrigger", RE::BSWin32GamepadDevice::Key::kLeftTrigger},
    {"righttrigger", RE::BSWin32GamepadDevice::Key::kRightTrigger},
};

void InputConfig::Install() {
    std::ifstream file(inputConfigFolder);
    if (!file.is_open()) {
        logger::error("Failed to open input configuration file: {}", inputConfigFolder);
        return;
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        logger::error("Error parsing JSON: {}", e.what());
        return;
    }

    logger::trace("Reading key bindings from: {}", inputConfigFolder);
    if (!j.is_object()) {
        logger::warn("Key bind JSON must be an object: {}", inputConfigFolder);
        return;
    }

    input_configs.clear();
    lastTapTimes.clear();

    for (const auto& [key, value] : j.items()) {
        std::string actionName = key;
        input_configs[actionName].clear();

        try {
            if (!value.is_array()) {
                logger::warn("Value for '{}' is not an array", actionName);
                continue;
            }

            for (const auto& item : value) {
                if (!item.is_object()) {
                    logger::warn("Item in array for '{}' is not an object", actionName);
                    continue;
                }

                std::string deviceStr = item.value("device", "");
                std::string keyStr = item.value("key", "");
                std::string triggerStr = item.value("trigger", "press");

                if (deviceStr.empty() || keyStr.empty()) {
                    logger::warn("Missing 'device' or 'key' in item for '{}'", actionName);
                    continue;
                }

                auto deviceIt = deviceMap.find(deviceStr);
                if (deviceIt == deviceMap.end()) {
                    logger::warn("Unknown device: {}", deviceStr);
                    continue;
                }

                RE::INPUT_DEVICE device = deviceIt->second;

                Trigger trigger = Trigger::kPress;
                if (triggerStr == "doubleTap") {
                    trigger = Trigger::kDoubleTap;
                } else if (triggerStr != "press") {
                    logger::warn("Unknown trigger '{}' in item for '{}'", triggerStr, actionName);
                    continue;
                }

                uint32_t keyCode = 0;

                if (device == RE::INPUT_DEVICE::kKeyboard) {
                    auto keyIt = keyboardMap.find(keyStr);
                    if (keyIt == keyboardMap.end()) {
                        logger::warn("Unknown keyboard key: {}", keyStr);
                        continue;
                    }
                    keyCode = static_cast<uint32_t>(keyIt->second);
                } else if (device == RE::INPUT_DEVICE::kMouse) {
                    auto keyIt = mouseMap.find(keyStr);
                    if (keyIt == mouseMap.end()) {
                        logger::warn("Unknown mouse key: {}", keyStr);
                        continue;
                    }
                    keyCode = static_cast<uint32_t>(keyIt->second);
                } else if (device == RE::INPUT_DEVICE::kGamepad) {
                    auto keyIt = gamepadMap.find(keyStr);
                    if (keyIt == gamepadMap.end()) {
                        logger::warn("Unknown gamepad key: {}", keyStr);
                        continue;
                    }
                    keyCode = static_cast<uint32_t>(keyIt->second);
                } else {
                    logger::warn("Unsupported device type: {}", deviceStr);
                    continue;
                }

                input_configs[actionName].push_back({device, keyCode, trigger});
            }
        } catch (const std::exception& e) {
            logger::error("Error processing action '{}': {}", actionName, e.what());
        }
    }
}
std::vector<std::pair<RE::INPUT_DEVICE, uint32_t>> InputConfig::Get(const std::string& key) {
    auto it = input_configs.find(key);
    if (it == input_configs.end()) {
        return {};
    }

    std::vector<std::pair<RE::INPUT_DEVICE, uint32_t>> bindings;
    bindings.reserve(it->second.size());
    for (const Binding& binding : it->second) {
        bindings.emplace_back(binding.device, binding.key);
    }
    return bindings;
}

bool InputConfig::IsActivated(const std::string& key, RE::InputEvent* event) {
    if (!event) {
        return false;
    }

    RE::ButtonEvent* button = event->AsButtonEvent();
    if (!button || !button->IsDown()) {
        return false;
    }

    const auto bindings = input_configs.find(key);
    if (bindings == input_configs.end()) {
        return false;
    }

    for (const Binding& binding : bindings->second) {
        if (event->GetDevice() != binding.device || button->GetIDCode() != binding.key) {
            continue;
        }

        if (binding.trigger == Trigger::kPress) {
            return true;
        }

        const TapKey tapKey{key, binding.device, binding.key};
        const TapClock::time_point now = TapClock::now();
        const auto lastTap = lastTapTimes.find(tapKey);
        if (lastTap != lastTapTimes.end() && now - lastTap->second <= doubleTapInterval) {
            lastTapTimes.erase(lastTap);
            return true;
        }

        lastTapTimes[tapKey] = now;
        return false;
    }

    return false;
}

namespace {
    std::string GetKeyLabel(RE::INPUT_DEVICE device, uint32_t key) {
        if (device == RE::INPUT_DEVICE::kKeyboard) {
            char keyName[64]{};
            const uint32_t scanCode = key & 0x7F;
            LONG keyData = static_cast<LONG>(scanCode << 16);
            if ((key & 0x80) != 0) {
                keyData |= 1 << 24;
            }
            if (GetKeyNameTextA(keyData, keyName, static_cast<int>(sizeof(keyName))) > 0) {
                return keyName;
            }
        } else if (device == RE::INPUT_DEVICE::kMouse) {
            switch (static_cast<RE::BSWin32MouseDevice::Key>(key)) {
                case RE::BSWin32MouseDevice::Key::kLeftButton:
                    return "Mouse 1";
                case RE::BSWin32MouseDevice::Key::kRightButton:
                    return "Mouse 2";
                case RE::BSWin32MouseDevice::Key::kMiddleButton:
                    return "Mouse 3";
                case RE::BSWin32MouseDevice::Key::kButton3:
                    return "Mouse 4";
                case RE::BSWin32MouseDevice::Key::kButton4:
                    return "Mouse 5";
                case RE::BSWin32MouseDevice::Key::kButton5:
                    return "Mouse 6";
                case RE::BSWin32MouseDevice::Key::kButton6:
                    return "Mouse 7";
                case RE::BSWin32MouseDevice::Key::kButton7:
                    return "Mouse 8";
                case RE::BSWin32MouseDevice::Key::kWheelUp:
                    return "Wheel Up";
                case RE::BSWin32MouseDevice::Key::kWheelDown:
                    return "Wheel Down";
                default:
                    break;
            }
        } else if (device == RE::INPUT_DEVICE::kGamepad) {
            switch (static_cast<RE::BSWin32GamepadDevice::Key>(key)) {
                case RE::BSWin32GamepadDevice::Key::kUp:
                    return "D-Pad Up";
                case RE::BSWin32GamepadDevice::Key::kDown:
                    return "D-Pad Down";
                case RE::BSWin32GamepadDevice::Key::kLeft:
                    return "D-Pad Left";
                case RE::BSWin32GamepadDevice::Key::kRight:
                    return "D-Pad Right";
                case RE::BSWin32GamepadDevice::Key::kStart:
                    return "Start";
                case RE::BSWin32GamepadDevice::Key::kBack:
                    return "Back";
                case RE::BSWin32GamepadDevice::Key::kLeftThumb:
                    return "Left Stick";
                case RE::BSWin32GamepadDevice::Key::kRightThumb:
                    return "Right Stick";
                case RE::BSWin32GamepadDevice::Key::kLeftShoulder:
                    return "Left Shoulder";
                case RE::BSWin32GamepadDevice::Key::kRightShoulder:
                    return "Right Shoulder";
                case RE::BSWin32GamepadDevice::Key::kA:
                    return "A";
                case RE::BSWin32GamepadDevice::Key::kB:
                    return "B";
                case RE::BSWin32GamepadDevice::Key::kX:
                    return "X";
                case RE::BSWin32GamepadDevice::Key::kY:
                    return "Y";
                case RE::BSWin32GamepadDevice::Key::kLeftTrigger:
                    return "Left Trigger";
                case RE::BSWin32GamepadDevice::Key::kRightTrigger:
                    return "Right Trigger";
                default:
                    break;
            }
        }

        return std::format("Key {}", key);
    }
}

std::string InputConfig::GetBindingLabel(const std::string& key) {
    const auto bindings = input_configs.find(key);
    if (bindings == input_configs.end()) {
        return {};
    }

    std::string label;

    for (const Binding& binding : bindings->second) {
        if (!label.empty()) {
            label += " / ";
        }

        const std::string keyLabel = GetKeyLabel(binding.device, binding.key);
        if (binding.trigger == Trigger::kDoubleTap) {
            label += std::vformat(
                Translations::Get("Input.Binding.DoubleTap"),
                std::make_format_args(keyLabel));
        } else {
            label += keyLabel;
        }
    }

    return label;
}
