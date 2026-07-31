#include "Icon.h"

#include "Input.h"
#include "Texture.h"

#define ICON_PATH(NAME) "Data\\Interface\\ImGuiIcons\\Icons\\" NAME ".png"
namespace IconPaths {

    const char* stepperLeft{ICON_PATH("StepperLeft")};
    const char* stepperRight{ICON_PATH("StepperRight")};
    const char* checkbox{ICON_PATH("Checkbox")};
    const char* checkboxFilled{ICON_PATH("Checkbox-Filled")};
    const char* unknownKey{ICON_PATH("UnknownKey")};

    std::map<Input::KeyboardKeys, const char*> keyboard{{Input::KeyboardKeys::kTab, ICON_PATH("Tab")},
                                                        {Input::KeyboardKeys::kUp, ICON_PATH("Up")},
                                                        {Input::KeyboardKeys::kDown, ICON_PATH("Down")},
                                                        {Input::KeyboardKeys::kLeft, ICON_PATH("Left")},
                                                        {Input::KeyboardKeys::kRight, ICON_PATH("Right")},

                                                        {Input::KeyboardKeys::kPageUp, ICON_PATH("PgUp")},
                                                        {Input::KeyboardKeys::kPageDown, ICON_PATH("PgDn")},
                                                        {Input::KeyboardKeys::kHome, ICON_PATH("Home")},
                                                        {Input::KeyboardKeys::kEnd, ICON_PATH("End")},
                                                        {Input::KeyboardKeys::kInsert, ICON_PATH("Insert")},
                                                        {Input::KeyboardKeys::kDelete, ICON_PATH("Delete")},
                                                        {Input::KeyboardKeys::kBackspace, ICON_PATH("Backspace")},
                                                        {Input::KeyboardKeys::kSpacebar, ICON_PATH("Space")},
                                                        {Input::KeyboardKeys::kEnter, ICON_PATH("Enter")},
                                                        {Input::KeyboardKeys::kEscape, ICON_PATH("Esc")},
                                                        {Input::KeyboardKeys::kLeftControl, ICON_PATH("L-Ctrl")},
                                                        {Input::KeyboardKeys::kLeftShift, ICON_PATH("L-Shift")},
                                                        {Input::KeyboardKeys::kLeftAlt, ICON_PATH("L-Alt")},
                                                        //{ Input::KeyboardKeys::kLeftWin, ICON_PATH("LeftWin") },
                                                        {Input::KeyboardKeys::kRightControl, ICON_PATH("R-Ctrl")},
                                                        {Input::KeyboardKeys::kRightShift, ICON_PATH("R-Shift")},
                                                        {Input::KeyboardKeys::kRightAlt, ICON_PATH("R-Alt")},
                                                        //{ Input::KeyboardKeys::kRightWin, ICON_PATH("RightWin") },
                                                        {Input::KeyboardKeys::kNum0, ICON_PATH("0")},
                                                        {Input::KeyboardKeys::kNum1, ICON_PATH("1")},
                                                        {Input::KeyboardKeys::kNum2, ICON_PATH("2")},
                                                        {Input::KeyboardKeys::kNum3, ICON_PATH("3")},
                                                        {Input::KeyboardKeys::kNum4, ICON_PATH("4")},
                                                        {Input::KeyboardKeys::kNum5, ICON_PATH("5")},
                                                        {Input::KeyboardKeys::kNum6, ICON_PATH("6")},
                                                        {Input::KeyboardKeys::kNum7, ICON_PATH("7")},
                                                        {Input::KeyboardKeys::kNum8, ICON_PATH("8")},
                                                        {Input::KeyboardKeys::kNum9, ICON_PATH("9")},
                                                        {Input::KeyboardKeys::kA, ICON_PATH("A")},
                                                        {Input::KeyboardKeys::kB, ICON_PATH("B")},
                                                        {Input::KeyboardKeys::kC, ICON_PATH("C")},
                                                        {Input::KeyboardKeys::kD, ICON_PATH("D")},
                                                        {Input::KeyboardKeys::kE, ICON_PATH("E")},
                                                        {Input::KeyboardKeys::kF, ICON_PATH("F")},
                                                        {Input::KeyboardKeys::kG, ICON_PATH("G")},
                                                        {Input::KeyboardKeys::kH, ICON_PATH("H")},
                                                        {Input::KeyboardKeys::kI, ICON_PATH("I")},
                                                        {Input::KeyboardKeys::kJ, ICON_PATH("J")},
                                                        {Input::KeyboardKeys::kK, ICON_PATH("K")},
                                                        {Input::KeyboardKeys::kL, ICON_PATH("L")},
                                                        {Input::KeyboardKeys::kM, ICON_PATH("M")},
                                                        {Input::KeyboardKeys::kN, ICON_PATH("N")},
                                                        {Input::KeyboardKeys::kO, ICON_PATH("O")},
                                                        {Input::KeyboardKeys::kP, ICON_PATH("P")},
                                                        {Input::KeyboardKeys::kQ, ICON_PATH("Q")},
                                                        {Input::KeyboardKeys::kR, ICON_PATH("R")},
                                                        {Input::KeyboardKeys::kS, ICON_PATH("S")},
                                                        {Input::KeyboardKeys::kT, ICON_PATH("T")},
                                                        {Input::KeyboardKeys::kU, ICON_PATH("U")},
                                                        {Input::KeyboardKeys::kV, ICON_PATH("V")},
                                                        {Input::KeyboardKeys::kW, ICON_PATH("W")},
                                                        {Input::KeyboardKeys::kX, ICON_PATH("X")},
                                                        {Input::KeyboardKeys::kY, ICON_PATH("Y")},
                                                        {Input::KeyboardKeys::kZ, ICON_PATH("Z")},
                                                        {Input::KeyboardKeys::kF1, ICON_PATH("F1")},
                                                        {Input::KeyboardKeys::kF2, ICON_PATH("F2")},
                                                        {Input::KeyboardKeys::kF3, ICON_PATH("F3")},
                                                        {Input::KeyboardKeys::kF4, ICON_PATH("F4")},
                                                        {Input::KeyboardKeys::kF5, ICON_PATH("F5")},
                                                        {Input::KeyboardKeys::kF6, ICON_PATH("F6")},
                                                        {Input::KeyboardKeys::kF7, ICON_PATH("F7")},
                                                        {Input::KeyboardKeys::kF8, ICON_PATH("F8")},
                                                        {Input::KeyboardKeys::kF9, ICON_PATH("F9")},
                                                        {Input::KeyboardKeys::kF10, ICON_PATH("F10")},
                                                        {Input::KeyboardKeys::kF11, ICON_PATH("F11")},
                                                        {Input::KeyboardKeys::kF12, ICON_PATH("F12")},
                                                        {Input::KeyboardKeys::kApostrophe, ICON_PATH("Quotesingle")},
                                                        {Input::KeyboardKeys::kComma, ICON_PATH("Comma")},
                                                        {Input::KeyboardKeys::kMinus, ICON_PATH("Hyphen")},
                                                        {Input::KeyboardKeys::kPeriod, ICON_PATH("Period")},
                                                        {Input::KeyboardKeys::kSlash, ICON_PATH("Slash")},
                                                        {Input::KeyboardKeys::kSemicolon, ICON_PATH("Semicolon")},
                                                        {Input::KeyboardKeys::kEquals, ICON_PATH("Equal")},
                                                        {Input::KeyboardKeys::kBracketLeft, ICON_PATH("Bracketleft")},
                                                        {Input::KeyboardKeys::kBackslash, ICON_PATH("Backslash")},
                                                        {Input::KeyboardKeys::kBracketRight, ICON_PATH("Bracketright")},
                                                        {Input::KeyboardKeys::kTilde, ICON_PATH("Tilde")},
                                                        {Input::KeyboardKeys::kCapsLock, ICON_PATH("CapsLock")},
                                                        {Input::KeyboardKeys::kScrollLock, ICON_PATH("ScrollLock")},
                                                        {Input::KeyboardKeys::kNumLock, ICON_PATH("NumLock")},
                                                        {Input::KeyboardKeys::kPrintScreen, ICON_PATH("PrintScreen")},
                                                        {Input::KeyboardKeys::kPause, ICON_PATH("Pause")},
                                                        {Input::KeyboardKeys::kKP_0, ICON_PATH("NumPad0")},
                                                        {Input::KeyboardKeys::kKP_1, ICON_PATH("Keypad1")},
                                                        {Input::KeyboardKeys::kKP_2, ICON_PATH("Keypad2")},
                                                        {Input::KeyboardKeys::kKP_3, ICON_PATH("Keypad3")},
                                                        {Input::KeyboardKeys::kKP_4, ICON_PATH("Keypad4")},
                                                        {Input::KeyboardKeys::kKP_5, ICON_PATH("Keypad5")},
                                                        {Input::KeyboardKeys::kKP_6, ICON_PATH("Keypad6")},
                                                        {Input::KeyboardKeys::kKP_7, ICON_PATH("Keypad7")},
                                                        {Input::KeyboardKeys::kKP_8, ICON_PATH("Keypad8")},
                                                        {Input::KeyboardKeys::kKP_9, ICON_PATH("NumPad9")},
                                                        {Input::KeyboardKeys::kKP_Decimal, ICON_PATH("NumPadDec")},
                                                        {Input::KeyboardKeys::kKP_Divide, ICON_PATH("NumPadDivide")},
                                                        {Input::KeyboardKeys::kKP_Multiply, ICON_PATH("NumPadMult")},
                                                        {Input::KeyboardKeys::kKP_Subtract, ICON_PATH("NumPadMinus")},
                                                        {Input::KeyboardKeys::kKP_Plus, ICON_PATH("NumPadPlus")},
                                                        {Input::KeyboardKeys::kKP_Enter, ICON_PATH("KeypadEnter")}};

    struct GamepadIcon {
        const char* xbox;
        const char* ps4;
    };

    std::map<std::uint32_t, GamepadIcon> gamePad{
        {static_cast<std::uint32_t>(Input::GamepadKeys::kStart), {ICON_PATH("360_Start"), ICON_PATH("PS3_Start")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kBack), {ICON_PATH("360_Back"), ICON_PATH("PS3_Back")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kLeftThumb), {ICON_PATH("360_L3"), ICON_PATH("PS3_L3")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kRightThumb), {ICON_PATH("360_R3"), ICON_PATH("PS3_R3")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kLeftShoulder), {ICON_PATH("360_LB"), ICON_PATH("PS3_LB")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kRightShoulder), {ICON_PATH("360_RB"), ICON_PATH("PS3_RB")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kA), {ICON_PATH("360_A"), ICON_PATH("PS3_A")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kB), {ICON_PATH("360_B"), ICON_PATH("PS3_B")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kX), {ICON_PATH("360_X"), ICON_PATH("PS3_X")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kY), {ICON_PATH("360_Y"), ICON_PATH("PS3_Y")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kLeftTrigger), {ICON_PATH("360_LT"), ICON_PATH("PS3_LT")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kRightTrigger), {ICON_PATH("360_RT"), ICON_PATH("PS3_RT")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kUp), {ICON_PATH("Up"), ICON_PATH("Up")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kDown), {ICON_PATH("Down"), ICON_PATH("Down")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kLeft), {ICON_PATH("Left"), ICON_PATH("Left")}},
        {static_cast<std::uint32_t>(Input::GamepadKeys::kRight), {ICON_PATH("Right"), ICON_PATH("Right")}},
    };

    std::map<std::uint32_t, const char*> mouse{
        {Input::MouseButtons::kLeftButton, ICON_PATH("Mouse1")}, {Input::MouseButtons::kRightButton, ICON_PATH("Mouse2")}, {Input::MouseButtons::kMiddleButton, ICON_PATH("Mouse3")},
        {Input::MouseButtons::kButton3, ICON_PATH("Mouse4")},    {Input::MouseButtons::kButton4, ICON_PATH("Mouse5 ")},    {Input::MouseButtons::kButton5, ICON_PATH("Mouse6")},
        {Input::MouseButtons::kButton6, ICON_PATH("Mouse7")},    {Input::MouseButtons::kButton7, ICON_PATH("Mouse8")},
    };

    const char* GetIconPath(Input::Source source, uint32_t key) {
        if (source == Input::Source::kKeyboard) {
            IF_FIND(keyboard, (Input::KeyboardKeys)key, it) { return it->second; }
        } else if (source == Input::Source::kMouseButton) {
            IF_FIND(mouse, (Input::MouseButtons)key, it) { return it->second; }
        } else if (source == Input::Source::kGamepadOrbis) {
            IF_FIND(gamePad, (Input::GamepadKeys)key, it) { return it->second.ps4; }
        }
        if (source == Input::Source::kGamepadDirectX) {
            IF_FIND(gamePad, (Input::GamepadKeys)key, it) { return it->second.xbox; }
        }
        return unknownKey;
    }
}
const char* Icons::GetPath(Input::Source device, uint32_t key) {
    return IconPaths::GetIconPath(device, key);
}

void Icons::Render(Input::Source device, uint32_t key, ImVec2 position, ImVec2 size, ImColor color) {
    TextureManager::Render(GetPath(device, key), position, size, color);
}
