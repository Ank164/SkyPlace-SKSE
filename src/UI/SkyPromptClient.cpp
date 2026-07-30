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
#include "Graphics.h"
#include "SlicedWindow.h"

#define PLACE_PLACE_BUTTON 1
#define PLACE_PICK_BUTTON 2
#define PLACE_TRANSFORM_BUTTON 3
#define PICK_MOVE_BUTTON 5
#define PICK_PICK_BUTTON 6
#define PICK_TOGGLE_SELECTION_BUTTON 10
#define PICK_DESELECT_ALL_BUTTON 11
#define INVENTORY_CLONE_BUTTON 12

using promptList = std::vector<SkyPromptAPI::Prompt>;

namespace {
    constexpr RE::FormID blackSoulGemFormID = 0x0002E504;
    constexpr std::uint32_t defaultPromptColor = 0xFFFFFFFF;
    constexpr std::uint32_t disabledPromptColor = IM_COL32(255, 0, 0, 255);
    constexpr std::uint32_t selectedPromptColor = IM_COL32(128, 255, 128, 255);
    constexpr float gamepadTranslationScale = 10.0f;
    constexpr float gamepadRotationScale = 5.0f;
    constexpr float gamepadObjectScale = 10.0f;

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
    std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>> buttons3 = InputConfig::Get("SkyPrompt.Place.Transform");

    promptList prompts = {SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Place.Place"), PLACE_PLACE_BUTTON, 0, SkyPromptAPI::PromptType::kHold, 0, buttons1, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Place.Pick"), PLACE_PICK_BUTTON, 0, SkyPromptAPI::PromptType::kHold, 0, buttons2, 0xFFFFFFFF),
                          SkyPromptAPI::Prompt(Translations::Get("SkyPrompt.Place.Transform"), PLACE_TRANSFORM_BUTTON, 0, SkyPromptAPI::PromptType::kSinglePress, 0, buttons3, 0xFFFFFFFF)};
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

enum class TransformMode {
    kTranslationUpDown,
    kTranslationLeftRight,
    kTranslationDepth,
    kRotationHorizontal,
    kRotationVertical,
    kRotationFree,
    kScale
};

bool isTransformMode = false;
bool isTransformDragging = false;
TransformMode transformMode = TransformMode::kTranslationDepth;
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
InventoryCloneSink* inventoryCloneSink = 0;

void ProcessTransformDelta(const RE::NiPoint2 delta) {
    RE::NiPoint2 constrainedDelta{};
    MenuEvent event = MenuEvent::kPlaceTranslateUpDown;

    switch (transformMode) {
        case TransformMode::kTranslationUpDown:
            constrainedDelta.y = delta.y;
            break;
        case TransformMode::kTranslationLeftRight:
            constrainedDelta.x = delta.x;
            event = MenuEvent::kPlaceTranslateLeftRight;
            break;
        case TransformMode::kTranslationDepth:
            constrainedDelta.y = delta.y;
            event = MenuEvent::kPlaceTranslateDepth;
            break;
        case TransformMode::kRotationHorizontal:
            constrainedDelta.x = delta.x;
            event = MenuEvent::kPlaceOrbitRotate;
            break;
        case TransformMode::kRotationVertical:
            constrainedDelta.y = delta.y;
            event = MenuEvent::kPlaceOrbitRotate;
            break;
        case TransformMode::kRotationFree:
            constrainedDelta = delta;
            event = MenuEvent::kPlaceOrbitRotate;
            break;
        case TransformMode::kScale:
            constrainedDelta.y = delta.y;
            event = MenuEvent::kPlaceScale;
            break;
    }

    if (constrainedDelta.x != 0.0f || constrainedDelta.y != 0.0f) {
        HUD::ProcessEvent(event, constrainedDelta);
    }
}

void SetTransformMode(bool value, bool showPlacementPrompts) {
    const bool wasTransformMode = isTransformMode;
    isTransformMode = value;
    isTransformDragging = false;

    if (ImGui::GetCurrentContext()) {
        ImGuiIO& io = ImGui::GetIO();
        if (value) {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        } else {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }
    }

    SkyPlaceCursorMenu::SetOpen(value);
    if (value) {
        transformMode = TransformMode::kTranslationDepth;
        HUD::ProcessEvent(MenuEvent::kPlaceSetRaycastDistance);
        if (placeSink) {
            placeSink->Hide();
        }
    } else if (wasTransformMode && showPlacementPrompts && placeSink) {
        placeSink->Show();
    }
}

bool RenderModeButton(
    const char* translationKey,
    const char* id,
    TransformMode mode,
    const ImVec2& size) {
    const bool selected = transformMode == mode;
    if (selected) {
        ImGui::PushStyleColor(ImGuiCol_Button, selectedPromptColor);
    }

    const std::string label = std::format(
        "{}##{}",
        Translations::Get(translationKey),
        id);
    const bool pressed = ImGui::Button(label.c_str(), size);
    if (selected) {
        ImGui::PopStyleColor();
    }
    if (pressed) {
        transformMode = mode;
        isTransformDragging = false;
        if (mode == TransformMode::kTranslationDepth) {
            HUD::ProcessEvent(MenuEvent::kPlaceSetRaycastDistance);
        }
    }
    return pressed;
}

void RenderTransformMenu() {
    if (!isTransformMode) {
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x - 40.0f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.5f));

    const std::string windowTitle = std::format(
        "{}###SkyPlaceTransformMenu",
        Translations::Get("TransformMenu.Title"));
    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings;

    if (SlicedWindow::Begin(windowTitle.c_str(), windowFlags)) {
        constexpr float buttonWidth = 360.0f;
        const ImVec2 buttonSize(buttonWidth, 0.0f);
        constexpr float fullWidth = buttonWidth;

        ImGui::TextUnformatted(Translations::Get("TransformMenu.Translation"));
        RenderModeButton(
            "TransformMenu.Translation.UpDown",
            "TranslationUpDown",
            TransformMode::kTranslationUpDown,
            buttonSize);
        RenderModeButton(
            "TransformMenu.Translation.LeftRight",
            "TranslationLeftRight",
            TransformMode::kTranslationLeftRight,
            buttonSize);
        RenderModeButton(
            "TransformMenu.Translation.Depth",
            "TranslationDepth",
            TransformMode::kTranslationDepth,
            buttonSize);

        ImGui::Separator();
        ImGui::TextUnformatted(Translations::Get("TransformMenu.Rotation"));
        RenderModeButton(
            "TransformMenu.Rotation.Horizontal",
            "RotationHorizontal",
            TransformMode::kRotationHorizontal,
            buttonSize);
        RenderModeButton(
            "TransformMenu.Rotation.Vertical",
            "RotationVertical",
            TransformMode::kRotationVertical,
            buttonSize);
        RenderModeButton(
            "TransformMenu.Rotation.Free",
            "RotationFree",
            TransformMode::kRotationFree,
            buttonSize);

        ImGui::Separator();
        RenderModeButton(
            "TransformMenu.Scale",
            "Scale",
            TransformMode::kScale,
            ImVec2(fullWidth, 0.0f));

        const std::string exitLabel = std::format(
            "{}##Exit",
            Translations::Get("TransformMenu.Exit"));
        if (ImGui::Button(exitLabel.c_str(), ImVec2(fullWidth, 0.0f))) {
            SetTransformMode(false, true);
        }
    }
    SlicedWindow::End();
}

void SkyPromptClient::Install() {
    clientID = SkyPromptAPI::RequestClientID();
    logger::trace("installing {}", clientID);
    placeSink = new PlaceSink();
    pickSink = new PickSink();
    inventoryCloneSink = new InventoryCloneSink();
    InputEventHandler::Register(OnInput);
    Graphics::Register(RenderTransformMenu);
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
            case PLACE_TRANSFORM_BUTTON:
                SetTransformMode(true, false);
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
        if (isTransformMode) {
            if (RE::ButtonEvent* button = event->AsButtonEvent();
                button && event->GetDevice() == RE::INPUT_DEVICE::kMouse) {
                if (button->GetIDCode() == RE::BSWin32MouseDevice::Key::kLeftButton) {
                    if (ImGui::GetCurrentContext()) {
                        ImGui::GetIO().AddMouseButtonEvent(
                            ImGuiMouseButton_Left,
                            button->IsPressed());
                    }
                    if (!button->IsPressed()) {
                        isTransformDragging = false;
                    } else {
                        const bool menuHasMouse = ImGui::GetCurrentContext() &&
                            ImGui::GetIO().WantCaptureMouse;
                        isTransformDragging = !menuHasMouse;
                    }
                }
                return true;
            }

            if (event->AsButtonEvent()) {
                return true;
            }

            if (RE::MouseMoveEvent* move = event->AsMouseMoveEvent()) {
                const RE::NiPoint2 delta{
                    static_cast<float>(move->mouseInputX),
                    static_cast<float>(move->mouseInputY)};
                if (isTransformDragging) {
                    ProcessTransformDelta(delta);
                }
                return false;
            }

            if (RE::ThumbstickEvent* move = event->AsThumbstickEvent()) {
                float inputScale = gamepadTranslationScale;
                if (transformMode == TransformMode::kRotationHorizontal ||
                    transformMode == TransformMode::kRotationVertical ||
                    transformMode == TransformMode::kRotationFree) {
                    inputScale = gamepadRotationScale;
                } else if (transformMode == TransformMode::kScale) {
                    inputScale = gamepadObjectScale;
                }
                ProcessTransformDelta(RE::NiPoint2{
                    move->xValue * inputScale,
                    -move->yValue * inputScale});
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

    SetTransformMode(false, false);
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
    SetTransformMode(false, false);
    for (auto& item : pickSink->set.prompts) {
        item.refid = ref->GetFormID();
    }
    pickSink->Hide();
    placeSink->Hide();
    pickSink->Show(handle);
}

void SkyPromptClient::ShowPlace() {
    if (!placeSink || !clientID) {
        return;
    }
    SetTransformMode(false, false);
    pickSink->Hide();
    placeSink->Show();
}

void SkyPromptClient::HidePlace() {
    if (!placeSink || !clientID) {
        return;
    }
    SetTransformMode(false, false);
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
