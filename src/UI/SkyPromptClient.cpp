#include "SkyPromptClient.h"
#include "HUD.h"
#include "InputConfig.h"
#include "SkyPromptAPI.h"
#include "Translations.h"
#include "InputEventHandler.h"
#include "ObjectGroup.h"
#include "Picker.h"
#include "SkyPlaceCursorMenu.h"
#include "SkyPlaceConfig.h"

#define PLACE_PLACE_BUTTON 1
#define PLACE_PICK_BUTTON 2
#define PLACE_TRANSLATE_BUTTON 3
#define PICK_MOVE_BUTTON 5
#define PICK_PICK_BUTTON 6
#define PICK_TOGGLE_SELECTION_BUTTON 10
#define PICK_DESELECT_ALL_BUTTON 11
#define INVENTORY_CLONE_BUTTON 12
#define PLACE_ORBIT_BUTTON 13
#define PLACE_EXIT_ROTATION_BUTTON 14
#define PLACE_ROTATION_HORIZONTAL_BUTTON 15
#define PLACE_ROTATION_VERTICAL_BUTTON 16
#define PLACE_ROTATION_FREE_BUTTON 17
#define PLACE_EXIT_TRANSLATION_BUTTON 18
#define PLACE_TRANSLATION_UP_DOWN_BUTTON 19
#define PLACE_TRANSLATION_LEFT_RIGHT_BUTTON 20
#define PLACE_TRANSLATION_DEPTH_BUTTON 21

using promptList = std::vector<SkyPromptAPI::Prompt>;

namespace {
    constexpr RE::FormID blackSoulGemFormID = 0x0002E504;
    constexpr std::uint32_t defaultPromptColor = 0xFFFFFFFF;
    constexpr std::uint32_t disabledPromptColor = IM_COL32(255, 0, 0, 255);
    constexpr std::uint32_t selectedPromptColor = IM_COL32(128, 255, 128, 255);
    constexpr float gamepadTranslationScale = 10.0f;
    constexpr float gamepadRotationScale = 5.0f;

    bool IsInventoryCloneItem(RE::TESBoundObject* item) {
        if (!item || !item->As<RE::TESObjectMISC>() || !IsDynamicId(item->GetFormID())) {
            return false;
        }

        return ObjectGroup::IsGroupItem(item);
    }

    std::int32_t GetInventoryCloneCost(RE::TESBoundObject* item) {
        if (!item) {
            return 1;
        }

        const std::map<RE::FormID, ObjectGroup::Data>& groups = ObjectGroup::GetAll();
        const auto group = groups.find(item->GetFormID());
        if (group == groups.end() || group->second.members.empty()) {
            return 1;
        }

        return static_cast<std::int32_t>(group->second.members.size());
    }

    RE::TESSoulGem* GetBlackSoulGem() {
        return RE::TESForm::LookupByID<RE::TESSoulGem>(blackSoulGemFormID);
    }
}

struct ButtonSetPlace {
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons1 = InputConfig::Get("SkyPrompt.Place.Place");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons2 = InputConfig::Get("SkyPrompt.Place.Pick");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons3 = InputConfig::Get("SkyPrompt.Place.Translate");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons4 = InputConfig::Get("SkyPrompt.Place.Orbit");

    promptList prompts = {SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Place.Place"), PLACE_PLACE_BUTTON, 0, SkyPromptAPI::PromptType::kHold, 0, buttons1, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Place.Pick"), PLACE_PICK_BUTTON, 0, SkyPromptAPI::PromptType::kHold, 0, buttons2, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Place.Translate"), PLACE_TRANSLATE_BUTTON, 0, SkyPromptAPI::PromptType::kSinglePress, 0, buttons3, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Place.Orbit"), PLACE_ORBIT_BUTTON, 0, SkyPromptAPI::PromptType::kSinglePress, 0, buttons4, 0xFFFFFFFF)};
};

struct ButtonSetRotation {
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> exitButtons =
        InputConfig::Get("SkyPrompt.Place.Orbit");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> horizontalButtons =
        InputConfig::Get("SkyPrompt.Place.RotationHorizontal");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> verticalButtons =
        InputConfig::Get("SkyPrompt.Place.RotationVertical");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> freeButtons =
        InputConfig::Get("SkyPrompt.Place.RotationFree");

    promptList prompts = {
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.ExitRotation"),
            PLACE_EXIT_ROTATION_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            exitButtons,
            0xFFFFFFFF),
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.RotationHorizontal"),
            PLACE_ROTATION_HORIZONTAL_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            horizontalButtons,
            0xFFFFFFFF),
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.RotationVertical"),
            PLACE_ROTATION_VERTICAL_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            verticalButtons,
            0xFFFFFFFF),
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.RotationFree"),
            PLACE_ROTATION_FREE_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            freeButtons,
            0xFFFFFFFF)
    };
};

struct ButtonSetTranslation {
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> exitButtons =
        InputConfig::Get("SkyPrompt.Place.Translate");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> depthButtons =
        InputConfig::Get("SkyPrompt.Place.TranslationDepth");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> upDownButtons =
        InputConfig::Get("SkyPrompt.Place.TranslationUpDown");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> leftRightButtons =
        InputConfig::Get("SkyPrompt.Place.TranslationLeftRight");

    promptList prompts = {
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.ExitTranslation"),
            PLACE_EXIT_TRANSLATION_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            exitButtons,
            0xFFFFFFFF),
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.TranslationDepth"),
            PLACE_TRANSLATION_DEPTH_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            depthButtons,
            0xFFFFFFFF),
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.TranslationUpDown"),
            PLACE_TRANSLATION_UP_DOWN_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            upDownButtons,
            0xFFFFFFFF),
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Place.TranslationLeftRight"),
            PLACE_TRANSLATION_LEFT_RIGHT_BUTTON,
            0,
            SkyPromptAPI::PromptType::kSinglePress,
            0,
            leftRightButtons,
            0xFFFFFFFF)
    };
};

struct ButtonSetPick {
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons1 = InputConfig::Get("SkyPrompt.Pick.Pick");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons2 = InputConfig::Get("SkyPrompt.Pick.Move");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons3 = InputConfig::Get("SkyPrompt.Pick.Select");
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons4 = InputConfig::Get("SkyPrompt.Pick.DeselectAll");

    promptList prompts = {SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Pick.Move"), PICK_MOVE_BUTTON, 0, SkyPromptAPI::PromptType::kHold, 0, buttons2, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Pick.Pick"), PICK_PICK_BUTTON, 0, SkyPromptAPI::PromptType::kHold, 0, buttons1, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Pick.Select"), PICK_TOGGLE_SELECTION_BUTTON, 0, SkyPromptAPI::PromptType::kSinglePress, 0, buttons3, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Pick.DeselectAll"), PICK_DESELECT_ALL_BUTTON, 0, SkyPromptAPI::PromptType::kHold, 0, buttons4, 0xFFFFFFFF)};
};

struct ButtonSetInventoryClone {
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons =
        InputConfig::Get("SkyPrompt.Inventory.Clone");
    promptList prompts = {
        SkyPromptAPI::Prompt(
            Translations::Get("SkyPrompt.Inventory.Clone.Single"),
            INVENTORY_CLONE_BUTTON,
            0,
            SkyPromptAPI::PromptType::kHold,
            0,
            buttons,
            0xFFFFFFFF)
    };
};


SkyPromptAPI::ClientID clientID = 0;

enum class OrbitRotationConstraint {
    kHorizontal,
    kVertical,
    kFree
};

enum class TranslationConstraint {
    kUpDown,
    kLeftRight,
    kDepth
};

bool is3DTranslationMode = false;
bool isTranslationDragging = false;
TranslationConstraint translationConstraint = TranslationConstraint::kDepth;
bool isOrbitRotationMode = false;
bool isOrbitDragging = false;
OrbitRotationConstraint orbitRotationConstraint = OrbitRotationConstraint::kHorizontal;
bool isEnabled = false;

class PlaceSink final : public SkyPromptAPI::PromptSink {
public:
    void Show() { SkyPromptAPI::SendPrompt(this, clientID); }

    void Hide() {
        SkyPromptAPI::RemovePrompt(this, clientID);
    }

    PlaceSink() { set = ButtonSetPlace(); };

    static inline ButtonSetPlace set;

    void ProcessEvent(const SkyPromptAPI::PromptEvent event) override;

    std::span<const SkyPromptAPI::Prompt> GetPrompts() override { return set.prompts; }
};

class PickSink final : public SkyPromptAPI::PromptSink {
public:
    void Show(const RE::ObjectRefHandle& hoverHandle) {
        RefreshPrompts(hoverHandle);
        SkyPromptAPI::SendPrompt(this, clientID);
    }

    PickSink() {
        set = ButtonSetPick();
    }

    void Hide() {
        SkyPromptAPI::RemovePrompt(this, clientID);
    }

    static inline ButtonSetPick set;
    std::map<std::size_t, std::string> deselectAllTexts;
    bool moveAndPickPromptsDisabled = false;

    void ProcessEvent(const SkyPromptAPI::PromptEvent event) override;

    void RefreshPrompts(const RE::ObjectRefHandle& hoverHandle) {
        const bool isHoverSelected = Picker::IsSelected(hoverHandle);
        moveAndPickPromptsDisabled = Picker::GetSelectionCount() > 0 && !isHoverSelected;

        const char* selectionText = isHoverSelected ?
            Translations::Get("SkyPrompt.Pick.Deselect") :
            Translations::Get("SkyPrompt.Pick.Select");
        set.prompts[2].text = selectionText;

        const std::uint32_t objectPromptColor =
            moveAndPickPromptsDisabled ? disabledPromptColor : defaultPromptColor;
        set.prompts[0].text_color = objectPromptColor;
        set.prompts[1].text_color = objectPromptColor;
        set.prompts[2].text_color = defaultPromptColor;

        const std::size_t selectionCount = Picker::GetSelectionCount();
        if (selectionCount > 0) {
            auto text = deselectAllTexts.find(selectionCount);
            if (text == deselectAllTexts.end()) {
                text = deselectAllTexts.emplace(
                    selectionCount,
                    fmt::format(
                        "{} ({})",
                        Translations::Get("SkyPrompt.Pick.DeselectAll"),
                        selectionCount)).first;
            }
            set.prompts[3].text = text->second;
            set.prompts[3].text_color = selectedPromptColor;
        } else {
            set.prompts[3].text = Translations::Get("SkyPrompt.Pick.DeselectAll");
            set.prompts[3].text_color = defaultPromptColor;
        }
    }

    std::span<const SkyPromptAPI::Prompt> GetPrompts() override { return set.prompts; }
};

class RotationSink final : public SkyPromptAPI::PromptSink {
public:
    void Show() {
        RefreshPrompts();
        SkyPromptAPI::SendPrompt(this, clientID);
    }

    void Hide() { SkyPromptAPI::RemovePrompt(this, clientID); }

    RotationSink() { set = ButtonSetRotation(); }

    static inline ButtonSetRotation set;

    void ProcessEvent(const SkyPromptAPI::PromptEvent event) override;

    void RefreshPrompts() {
        set.prompts[1].text_color =
            orbitRotationConstraint == OrbitRotationConstraint::kHorizontal ?
                selectedPromptColor : defaultPromptColor;
        set.prompts[2].text_color =
            orbitRotationConstraint == OrbitRotationConstraint::kVertical ?
                selectedPromptColor : defaultPromptColor;
        set.prompts[3].text_color =
            orbitRotationConstraint == OrbitRotationConstraint::kFree ?
                selectedPromptColor : defaultPromptColor;
    }

    std::span<const SkyPromptAPI::Prompt> GetPrompts() override { return set.prompts; }
};

class TranslationSink final : public SkyPromptAPI::PromptSink {
public:
    void Show() {
        RefreshPrompts();
        SkyPromptAPI::SendPrompt(this, clientID);
    }

    void Hide() { SkyPromptAPI::RemovePrompt(this, clientID); }

    TranslationSink() { set = ButtonSetTranslation(); }

    static inline ButtonSetTranslation set;

    void ProcessEvent(const SkyPromptAPI::PromptEvent event) override;

    void RefreshPrompts() {
        set.prompts[1].text_color =
            translationConstraint == TranslationConstraint::kDepth ?
                selectedPromptColor : defaultPromptColor;
        set.prompts[2].text_color =
            translationConstraint == TranslationConstraint::kUpDown ?
                selectedPromptColor : defaultPromptColor;
        set.prompts[3].text_color =
            translationConstraint == TranslationConstraint::kLeftRight ?
                selectedPromptColor : defaultPromptColor;
    }

    std::span<const SkyPromptAPI::Prompt> GetPrompts() override { return set.prompts; }
};

class InventoryCloneSink final : public SkyPromptAPI::PromptSink {
public:
    void Show() { SkyPromptAPI::SendPrompt(this, clientID); }

    InventoryCloneSink() { set = ButtonSetInventoryClone(); }

    void Hide() {
        SkyPromptAPI::RemovePrompt(this, clientID);
        itemFormID = 0;
        cloneCost = 0;
        promptText.clear();
    }

    static inline ButtonSetInventoryClone set;
    RE::FormID itemFormID = 0;
    std::int32_t cloneCost = 0;
    std::string promptText;

    void ProcessEvent(const SkyPromptAPI::PromptEvent event) override;

    std::span<const SkyPromptAPI::Prompt> GetPrompts() override { return set.prompts; }
};

PlaceSink* placeSink = 0;
PickSink* pickSink = 0;
RotationSink* rotationSink = 0;
TranslationSink* translationSink = 0;
InventoryCloneSink* inventoryCloneSink = 0;

void Process3DTranslationDelta(const RE::NiPoint2 delta) {
    RE::NiPoint2 constrainedDelta{};
    MenuEvent event = MenuEvent::kPlaceTranslateUpDown;
    switch (translationConstraint) {
        case TranslationConstraint::kUpDown:
            constrainedDelta.y = delta.y;
            break;
        case TranslationConstraint::kLeftRight:
            constrainedDelta.x = delta.x;
            event = MenuEvent::kPlaceTranslateLeftRight;
            break;
        case TranslationConstraint::kDepth:
            constrainedDelta.y = delta.y;
            event = MenuEvent::kPlaceTranslateDepth;
            break;
    }

    if (constrainedDelta.x != 0.0f || constrainedDelta.y != 0.0f) {
        HUD::ProcessEvent(event, constrainedDelta);
    }
}

void ProcessOrbitRotationDelta(const RE::NiPoint2 delta) {
    RE::NiPoint2 rotationDelta{};
    switch (orbitRotationConstraint) {
        case OrbitRotationConstraint::kHorizontal:
            rotationDelta.x = delta.x;
            break;
        case OrbitRotationConstraint::kVertical:
            rotationDelta.y = delta.y;
            break;
        case OrbitRotationConstraint::kFree:
            rotationDelta = delta;
            break;
    }

    if (rotationDelta.x == 0.0f && rotationDelta.y == 0.0f) {
        return;
    }

    HUD::ProcessEvent(
        MenuEvent::kPlaceOrbitRotate,
        rotationDelta);
}

void SetOrbitRotationMode(bool value, bool showPlacementPrompts) {
    const bool wasOrbitRotationMode = isOrbitRotationMode;
    isOrbitRotationMode = value;
    isOrbitDragging = false;
    const bool cursorEnabled = value || is3DTranslationMode;
    SkyPlaceCursorMenu::SetOpen(cursorEnabled);

    if (value) {
        orbitRotationConstraint = OrbitRotationConstraint::kHorizontal;
        placeSink->Hide();
        rotationSink->Show();
    } else {
        if (wasOrbitRotationMode) {
            rotationSink->Hide();
        }
        if (showPlacementPrompts) {
            placeSink->Show();
        }
    }
}

void Set3DTranslationMode(bool value, bool showPlacementPrompts) {
    const bool was3DTranslationMode = is3DTranslationMode;
    is3DTranslationMode = value;
    isTranslationDragging = false;
    const bool cursorEnabled = value || isOrbitRotationMode;
    SkyPlaceCursorMenu::SetOpen(cursorEnabled);

    if (value) {
        translationConstraint = TranslationConstraint::kDepth;
        HUD::ProcessEvent(MenuEvent::kPlaceSetRaycastDistance);
        placeSink->Hide();
        translationSink->Show();
    } else {
        if (was3DTranslationMode) {
            translationSink->Hide();
        }
        if (showPlacementPrompts) {
            placeSink->Show();
        }
    }
}

void SkyPromptClient::Install() {
    clientID = SkyPromptAPI::RequestClientID();
    logger::trace("installing {}", clientID);
    placeSink = new PlaceSink();
    pickSink = new PickSink();
    rotationSink = new RotationSink();
    translationSink = new TranslationSink();
    inventoryCloneSink = new InventoryCloneSink();
    InputEventHandler::Register(OnInput);
}


std::vector<std::string> map = {"kAccepted", "kDeclined", "kRemovedByMod", "kTimingOut", "kTimeout", "kDown", "kUp", "kMove"};

void PlaceSink::ProcessEvent(const SkyPromptAPI::PromptEvent event) {
    if (event.type == SkyPromptAPI::PromptEventType::kTimeout) {
        Show();
        return;
    }
    if (event.type == SkyPromptAPI::PromptEventType::kDeclined || event.type == SkyPromptAPI::PromptEventType::kRemovedByMod) {
        HUD::ProcessEvent(MenuEvent::kPlacerClose);
        Hide();
        return;
    }

    if (event.type == SkyPromptAPI::PromptEventType::kAccepted) {
        switch (event.prompt.eventID) {
            case PLACE_PICK_BUTTON:
                HUD::ProcessEvent(MenuEvent::kPlacePickAccepted);
                return;
            case PLACE_PLACE_BUTTON:
                HUD::ProcessEvent(MenuEvent::kPlacePlaceAccepted);
                return;
            case PLACE_ORBIT_BUTTON:
                SetOrbitRotationMode(true, false);
                return;
            case PLACE_TRANSLATE_BUTTON:
                Set3DTranslationMode(true, false);
                return;
            default:
                break;
        }
    }
}

void TranslationSink::ProcessEvent(const SkyPromptAPI::PromptEvent event) {
    if (event.type == SkyPromptAPI::PromptEventType::kTimeout) {
        if (is3DTranslationMode) {
            Show();
        }
        return;
    }

    if (event.type == SkyPromptAPI::PromptEventType::kDeclined ||
        event.type == SkyPromptAPI::PromptEventType::kRemovedByMod) {
        if (!is3DTranslationMode) {
            return;
        }
        is3DTranslationMode = false;
        isTranslationDragging = false;
        SkyPlaceCursorMenu::SetOpen(false);
        HUD::ProcessEvent(MenuEvent::kPlacerClose);
        Hide();
        return;
    }

    if (event.type == SkyPromptAPI::PromptEventType::kAccepted) {
        switch (event.prompt.eventID) {
            case PLACE_EXIT_TRANSLATION_BUTTON:
                Set3DTranslationMode(false, true);
                return;
            case PLACE_TRANSLATION_UP_DOWN_BUTTON:
                translationConstraint = TranslationConstraint::kUpDown;
                Hide();
                Show();
                return;
            case PLACE_TRANSLATION_LEFT_RIGHT_BUTTON:
                translationConstraint = TranslationConstraint::kLeftRight;
                Hide();
                Show();
                return;
            case PLACE_TRANSLATION_DEPTH_BUTTON:
                translationConstraint = TranslationConstraint::kDepth;
                HUD::ProcessEvent(MenuEvent::kPlaceSetRaycastDistance);
                Hide();
                Show();
                return;
            default:
                break;
        }
    }
}

void RotationSink::ProcessEvent(const SkyPromptAPI::PromptEvent event) {
    if (event.type == SkyPromptAPI::PromptEventType::kTimeout) {
        if (isOrbitRotationMode) {
            Show();
        }
        return;
    }

    if (event.type == SkyPromptAPI::PromptEventType::kDeclined ||
        event.type == SkyPromptAPI::PromptEventType::kRemovedByMod) {
        if (!isOrbitRotationMode) {
            return;
        }
        isOrbitRotationMode = false;
        isOrbitDragging = false;
        SkyPlaceCursorMenu::SetOpen(false);
        HUD::ProcessEvent(MenuEvent::kPlacerClose);
        Hide();
        return;
    }

    if (event.type == SkyPromptAPI::PromptEventType::kAccepted) {
        switch (event.prompt.eventID) {
            case PLACE_EXIT_ROTATION_BUTTON:
                SetOrbitRotationMode(false, true);
                return;
            case PLACE_ROTATION_HORIZONTAL_BUTTON:
                orbitRotationConstraint = OrbitRotationConstraint::kHorizontal;
                Hide();
                Show();
                return;
            case PLACE_ROTATION_VERTICAL_BUTTON:
                orbitRotationConstraint = OrbitRotationConstraint::kVertical;
                Hide();
                Show();
                return;
            case PLACE_ROTATION_FREE_BUTTON:
                orbitRotationConstraint = OrbitRotationConstraint::kFree;
                Hide();
                Show();
                return;
            default:
                break;
        }
    }
}

void SkyPromptClient::SetIsEnabled(bool value) { isEnabled = value; }

bool SkyPromptClient::GetIsEnabled() { return isEnabled; }


bool SkyPromptClient::OnInput(RE::InputEvent* event) {
    if (event) {
        if (isOrbitRotationMode || is3DTranslationMode) {
            if (RE::ButtonEvent* button = event->AsButtonEvent();
                button && event->GetDevice() == RE::INPUT_DEVICE::kMouse) {
                if (button->GetIDCode() == RE::BSWin32MouseDevice::Key::kLeftButton) {
                    if (isOrbitRotationMode) {
                        isOrbitDragging = button->IsPressed();
                    }
                    if (is3DTranslationMode) {
                        isTranslationDragging = button->IsPressed();
                    }
                }
                return true;
            }

            if (RE::MouseMoveEvent* move = event->AsMouseMoveEvent()) {
                const RE::NiPoint2 delta{
                    static_cast<float>(move->mouseInputX),
                    static_cast<float>(move->mouseInputY)};
                if (isOrbitDragging) {
                    ProcessOrbitRotationDelta(delta);
                }
                if (isTranslationDragging) {
                    Process3DTranslationDelta(delta);
                }
                return false;
            }
        }

        if (RE::ThumbstickEvent* move = event->AsThumbstickEvent()) {
            if (is3DTranslationMode) {
                Process3DTranslationDelta(RE::NiPoint2{
                    move->xValue * gamepadTranslationScale,
                    -move->yValue * gamepadTranslationScale});
                return true;
            }

            if (isOrbitRotationMode) {
                ProcessOrbitRotationDelta(RE::NiPoint2{
                    move->xValue * gamepadRotationScale,
                    -move->yValue * gamepadRotationScale});
                return true;
            }
        }
    }
    return false;
}

void SkyPromptClient::ShowInventoryClone(
    const RE::ObjectRefHandle& handle,
    RE::TESBoundObject* item)
{
    const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
    if (SkyPlaceConfig::IsSoulGemCloningDisabled() ||
        !inventoryCloneSink || !clientID || !ref || !IsInventoryCloneItem(item))
    {
        HideInventoryClone();
        return;
    }

    RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
    RE::TESSoulGem* blackSoulGem = GetBlackSoulGem();
    const std::int32_t cloneCost = GetInventoryCloneCost(item);
    if (!player || !blackSoulGem || player->GetItemCount(item) <= 0 ||
        player->GetItemCount(blackSoulGem) < cloneCost)
    {
        HideInventoryClone();
        return;
    }

    SetOrbitRotationMode(false, false);
    Set3DTranslationMode(false, false);
    inventoryCloneSink->Hide();
    inventoryCloneSink->itemFormID = item->GetFormID();
    inventoryCloneSink->cloneCost = cloneCost;
    if (cloneCost == 1) {
        inventoryCloneSink->promptText =
            Translations::Get("SkyPrompt.Inventory.Clone.Single");
    } else {
        inventoryCloneSink->promptText = std::vformat(
            Translations::Get("SkyPrompt.Inventory.Clone.Group"),
            std::make_format_args(cloneCost));
    }
    inventoryCloneSink->set.prompts[0].text = inventoryCloneSink->promptText;
    for (SkyPromptAPI::Prompt& prompt : inventoryCloneSink->set.prompts) {
        prompt.refid = ref->GetFormID();
    }

    pickSink->Hide();
    placeSink->Hide();
    inventoryCloneSink->Show();
}

void SkyPromptClient::ShowPick(const RE::ObjectRefHandle& handle) {
    const RE::NiPointer<RE::TESObjectREFR> ref = handle.get();
    if (!placeSink || !clientID || !ref) {
        return;
    }
    SetOrbitRotationMode(false, false);
    Set3DTranslationMode(false, false);
    for (auto& item : pickSink->set.prompts) {
        item.refid = ref->GetFormID();
    }
    pickSink->Hide();
    placeSink->Hide();
    pickSink->Show(handle);
}

void SkyPromptClient::ShowPlace() {
    if (!placeSink || !rotationSink || !translationSink || !clientID) {
        return;
    }
    SetOrbitRotationMode(false, false);
    Set3DTranslationMode(false, false);
    pickSink->Hide();
    placeSink->Show();
}

void SkyPromptClient::HidePlace() {
    if (!placeSink || !rotationSink || !translationSink || !clientID) {
        return;
    }
    SetOrbitRotationMode(false, false);
    Set3DTranslationMode(false, false);
    placeSink->Hide();
}

void SkyPromptClient::HidePick() {
    if (!placeSink || !clientID) {
        return;
    }

    pickSink->Hide();
}

void SkyPromptClient::HideInventoryClone() {
    if (inventoryCloneSink) {
        inventoryCloneSink->Hide();
    }
}

void PickSink::ProcessEvent(const SkyPromptAPI::PromptEvent event) {
    if (event.type == SkyPromptAPI::PromptEventType::kDeclined || event.type == SkyPromptAPI::PromptEventType::kRemovedByMod || event.type == SkyPromptAPI::PromptEventType::kTimeout) {
        HUD::ProcessEvent(MenuEvent::kClosePick);
        Hide();
        return;
    }

    if (event.type == SkyPromptAPI::PromptEventType::kAccepted) {
        if (moveAndPickPromptsDisabled &&
            (event.prompt.eventID == PICK_MOVE_BUTTON ||
             event.prompt.eventID == PICK_PICK_BUTTON))
        {
            return;
        }

        switch (event.prompt.eventID) {
            case PICK_PICK_BUTTON:
                HUD::ProcessEvent(MenuEvent::kPickPickAccepted);
                Hide();
                return;
            case PICK_MOVE_BUTTON:
                HUD::ProcessEvent(MenuEvent::kPickMoveAccepted);
                Hide();
                return;
            case PICK_TOGGLE_SELECTION_BUTTON:
                HUD::ProcessEvent(MenuEvent::kPickToggleSelectionAccepted);
                Hide();
                Show(Picker::GetLastHoverHandle());
                return;
            case PICK_DESELECT_ALL_BUTTON:
                HUD::ProcessEvent(MenuEvent::kPickDeselectAllAccepted);
                Hide();
                Show(Picker::GetLastHoverHandle());
                return;
            default:
                break;
        }
    }
}
void RefreshOpenPlayerInventory(RE::TESBoundObject* item) {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player || !item) {
        return;
    }

    auto ui = RE::UI::GetSingleton();
    if (!ui || !ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME)) {
        return;
    }

    auto queue = RE::UIMessageQueue::GetSingleton();
    if (!queue) {
        return;
    }

    auto data = static_cast<RE::InventoryUpdateData*>(queue->CreateUIMessageData("InventoryUpdateData"));
    if (!data) {
        return;
    }

    RE::CreateRefHandle(data->inventoryRef, player);
    data->updateObj = item;

    queue->AddMessage(RE::InventoryMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kInventoryUpdate, data);
}
void InventoryCloneSink::ProcessEvent(const SkyPromptAPI::PromptEvent event) {
    if (event.type != SkyPromptAPI::PromptEventType::kAccepted ||
        event.prompt.eventID != INVENTORY_CLONE_BUTTON)
    {
        return;
    }

    RE::PlayerCharacter* player = RE::PlayerCharacter::GetSingleton();
    RE::TESBoundObject* item =
        RE::TESForm::LookupByID<RE::TESBoundObject>(itemFormID);
    RE::TESSoulGem* blackSoulGem = GetBlackSoulGem();
    const std::int32_t currentCloneCost = GetInventoryCloneCost(item);
    if (SkyPlaceConfig::IsSoulGemCloningDisabled() ||
        !player || !blackSoulGem || !IsInventoryCloneItem(item) ||
        player->GetItemCount(item) <= 0 ||
        currentCloneCost != cloneCost ||
        player->GetItemCount(blackSoulGem) < currentCloneCost)
    {
        Hide();
        return;
    }

    RE::TESBoundObject* clonedItem = ObjectGroup::CloneEmpty(item);
    if (!clonedItem) {
        Hide();
        return;
    }

    player->RemoveItem(
        blackSoulGem,
        currentCloneCost,
        RE::ITEM_REMOVE_REASON::kRemove,
        nullptr,
        nullptr);
    player->AddObjectToContainer(clonedItem, nullptr, 1, nullptr);
    RE::UI::GetSingleton()->GetMenu<RE::InventoryMenu>()->itemList->Update();

    Hide();
}
