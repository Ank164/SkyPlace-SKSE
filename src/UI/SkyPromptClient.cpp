#include "SkyPromptClient.h"

#include <algorithm>
#include <array>

#include "HUD.h"
#include "InputConfig.h"
#include "SkyPromptAPI.h"
#include "Translations.h"
#include "InputEventHandler.h"
#include "Menu.h"
#include "ObjectGroup.h"
#include "Picker.h"
#include "Placer.h"
#include "SkyPlaceCursorMenu.h"
#include "SkyPlaceConfig.h"
#include "Graphics.h"
#include "SlicedWindow.h"
#include "Texture.h"

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

    constexpr char iconRotate[] = "\xEF\x80\xA1";
    constexpr char iconRotateLeft[] = "\xEF\x8B\xAA";
    constexpr char iconRotateRight[] = "\xEF\x8B\xB9";
    constexpr char iconUpDown[] = "\xEF\x8C\xB8";
    constexpr char iconLeftRight[] = "\xEF\x8C\xB7";
    constexpr char iconMove[] = "\xEF\x81\x87";
    constexpr char iconDepth[] = "\xEF\x86\xB2";
    constexpr char iconScale[] = "\xEF\x90\xA4";
    constexpr char iconExit[] = "\xEF\x8B\xB5";

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
    kTranslationForwardBackward,
    kTranslationDepth,
    kRotationHorizontal,
    kRotationVertical,
    kRotationFree,
    kScale
};

constexpr std::array transformModeOrder{
    TransformMode::kRotationHorizontal,
    TransformMode::kRotationVertical,
    TransformMode::kRotationFree,
    TransformMode::kTranslationUpDown,
    TransformMode::kTranslationLeftRight,
    TransformMode::kTranslationForwardBackward,
    TransformMode::kTranslationDepth,
    TransformMode::kScale};

bool isTransformMode = false;
bool isTransformDragging = false;
TransformMode transformMode = TransformMode::kRotationHorizontal;
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
        case TransformMode::kTranslationForwardBackward:
            constrainedDelta.y = delta.y;
            event = MenuEvent::kPlaceTranslateForwardBackward;
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

    if (isTransformMode == value) {
        return;
    }

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
        transformMode = TransformMode::kRotationHorizontal;
        HUD::ProcessEvent(MenuEvent::kPlaceSetRaycastDistance);
        if (placeSink) {
            placeSink->Hide();
        }
    } else if (wasTransformMode && showPlacementPrompts && placeSink) {
        placeSink->Show();
    }
}

void SetSelectedTransformMode(TransformMode mode) {
    transformMode = mode;
    isTransformDragging = false;
    if (mode == TransformMode::kTranslationDepth) {
        HUD::ProcessEvent(MenuEvent::kPlaceSetRaycastDistance);
    }
}

void CycleTransformMode(bool forward) {
    const auto current = std::find(
        transformModeOrder.begin(),
        transformModeOrder.end(),
        transformMode);
    std::size_t index = current == transformModeOrder.end() ? 0 :
        static_cast<std::size_t>(std::distance(transformModeOrder.begin(), current));

    if (forward) {
        index = (index + 1) % transformModeOrder.size();
    } else {
        index = (index + transformModeOrder.size() - 1) % transformModeOrder.size();
    }
    SetSelectedTransformMode(transformModeOrder[index]);
}

bool RenderTransformButton(
    const char* label,
    const char* icon,
    const char* id,
    bool selected,
    const ImVec2& size,
    float scale) {
    const ImVec2 labelSize = ImGui::CalcTextSize(label);
    const ImVec2 iconSize = ImGui::CalcTextSize(icon);
    const float iconSpacing = 12.0f * scale;
    const ImVec2 textSize{
        iconSize.x + iconSpacing + labelSize.x,
        std::max(iconSize.y, labelSize.y)};
    const float rowHeight = std::max(
        size.y,
        std::max(
            32.0f * scale,
            textSize.y + ImGui::GetStyle().FramePadding.y * 2.0f));
    const float iconWidth = 35.0f * scale;
    const float iconHeight = 28.0f * scale;

    const ImVec2 buttonMinimum = ImGui::GetCursorScreenPos();
    const ImVec2 buttonMaximum{
        buttonMinimum.x + size.x,
        buttonMinimum.y + rowHeight};
    const ImVec2 buttonSize{
        buttonMaximum.x - buttonMinimum.x,
        rowHeight};
    const std::string buttonID = std::format("##{}", id);
    ImGui::SetCursorScreenPos(buttonMinimum);
    const bool pressed = ImGui::InvisibleButton(
        buttonID.c_str(),
        buttonSize);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    constexpr ImU32 opaqueWhite = IM_COL32(255, 255, 255, 255);
    constexpr ImU32 transparentWhite = IM_COL32(255, 255, 255, 0);
    if (selected) {
        drawList->AddRectFilledMultiColor(
            buttonMinimum,
            buttonMaximum,
            transparentWhite,
            opaqueWhite,
            opaqueWhite,
            transparentWhite);
    }

    const float iconTop = buttonMinimum.y + (rowHeight - iconHeight) * 0.5f;
    const ImVec2 iconMinimum{
        buttonMinimum.x + ImGui::GetStyle().FramePadding.x,
        iconTop};
    const ImVec2 iconMaximum{
        iconMinimum.x + iconWidth,
        iconMinimum.y + iconHeight};

    if (selected) {
        const ImTextureID itemTexture = TextureManager::GetTexture(
            "Data\\SKSE\\Plugins\\SkyPlaceAssets\\item.svg",
            ImVec2{iconWidth, iconHeight});
        if (itemTexture) {
            drawList->AddImage(
                itemTexture,
                iconMinimum,
                iconMaximum);
        }
    }

    const ImVec2 iconPosition{
        buttonMinimum.x + (buttonSize.x - textSize.x) * 0.5f,
        buttonMinimum.y + (rowHeight - iconSize.y) * 0.5f};
    const ImU32 textColor = selected ?
        IM_COL32(0, 0, 0, 255) :
        ImGui::GetColorU32(ImGuiCol_Text);
    drawList->AddText(
        iconPosition,
        textColor,
        icon);
    drawList->AddText(
        ImVec2{
            iconPosition.x + iconSize.x + iconSpacing,
            buttonMinimum.y + (rowHeight - labelSize.y) * 0.5f},
        textColor,
        label);

    return pressed;
}

bool RenderModeButton(
    const char* translationKey,
    const char* icon,
    const char* id,
    TransformMode mode,
    const ImVec2& size,
    float scale) {
    const bool pressed = RenderTransformButton(
        Translations::Get(translationKey),
        icon,
        id,
        transformMode == mode,
        size,
        scale);

    if (pressed) {
        SetSelectedTransformMode(mode);
    }
    return pressed;
}

void RenderTransformMenu() {
    if (!isTransformMode) {
        return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    constexpr float referenceHeight = 1080.0f;
    const float scale = std::max(io.DisplaySize.y / referenceHeight, 0.01f);
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x - 40.0f * scale, io.DisplaySize.y * 0.5f),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.5f));
    const float windowWidth = 480.0f * scale;
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(windowWidth, 0.0f),
        ImVec2(windowWidth, io.DisplaySize.y));

    const std::string windowTitle = std::format(
        "{}###SkyPlaceTransformMenu",
        Translations::Get("TransformMenu.Title"));
    constexpr ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings;

    const ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(
        ImGuiStyleVar_FramePadding,
        ImVec2{style.FramePadding.x * scale, style.FramePadding.y * scale});
    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemSpacing,
        ImVec2{style.ItemSpacing.x * scale, style.ItemSpacing.y * scale});
    ImGui::PushStyleVar(
        ImGuiStyleVar_ItemInnerSpacing,
        ImVec2{style.ItemInnerSpacing.x * scale, style.ItemInnerSpacing.y * scale});

    ImFont* transformMenuFont = Graphics::GetTransformMenuFont();
    if (transformMenuFont) {
        ImGui::PushFont(transformMenuFont);
    }

    if (SlicedWindow::Begin(windowTitle.c_str(), windowFlags, scale)) {
        const float fullWidth = ImGui::GetContentRegionAvail().x;
        const ImVec2 buttonSize(fullWidth, 0.0f);

        ImGui::TextUnformatted(Translations::Get("TransformMenu.Rotation"));
        RenderModeButton(
            "TransformMenu.Rotation.Horizontal",
            iconRotateLeft,
            "RotationHorizontal",
            TransformMode::kRotationHorizontal,
            buttonSize,
            scale);
        RenderModeButton(
            "TransformMenu.Rotation.Vertical",
            iconRotateRight,
            "RotationVertical",
            TransformMode::kRotationVertical,
            buttonSize,
            scale);
        RenderModeButton(
            "TransformMenu.Rotation.Free",
            iconRotate,
            "RotationFree",
            TransformMode::kRotationFree,
            buttonSize,
            scale);

        ImGui::Separator();
        ImGui::TextUnformatted(Translations::Get("TransformMenu.Translation"));
        RenderModeButton(
            "TransformMenu.Translation.UpDown",
            iconUpDown,
            "TranslationUpDown",
            TransformMode::kTranslationUpDown,
            buttonSize,
            scale);
        RenderModeButton(
            "TransformMenu.Translation.LeftRight",
            iconLeftRight,
            "TranslationLeftRight",
            TransformMode::kTranslationLeftRight,
            buttonSize,
            scale);
        RenderModeButton(
            "TransformMenu.Translation.ForwardBackward",
            iconMove,
            "TranslationForwardBackward",
            TransformMode::kTranslationForwardBackward,
            buttonSize,
            scale);
        RenderModeButton(
            "TransformMenu.Translation.Depth",
            iconDepth,
            "TranslationDepth",
            TransformMode::kTranslationDepth,
            buttonSize,
            scale);

        ImGui::Separator();
        ImGui::TextUnformatted(Translations::Get("TransformMenu.Scale.Section"));
        RenderModeButton(
            "TransformMenu.Scale",
            iconScale,
            "Scale",
            TransformMode::kScale,
            ImVec2(fullWidth, 0.0f),
            scale);

        ImGui::Separator();
        ImGui::TextUnformatted(Translations::Get("TransformMenu.Exit.Section"));
        if (RenderTransformButton(
                Translations::Get("TransformMenu.Exit"),
                iconExit,
                "Exit",
                false,
                ImVec2(fullWidth, 0.0f),
                scale)) {
            SetTransformMode(false, true);
        }

        ImGui::Separator();
        ImGui::TextUnformatted(Translations::Get("TransformMenu.Controls.Section"));
        ImGui::PushTextWrapPos(0.0f);
        ImGui::BulletText(
            "%s",
            Translations::Get("TransformMenu.Controls.Select"));
        ImGui::BulletText(
            "%s",
            Translations::Get("TransformMenu.Controls.Cycle"));
        std::string transformBindingLabel =
            InputConfig::GetBindingLabel("SkyPrompt.Place.Transform");
        const std::string closeTip = std::vformat(
            Translations::Get("TransformMenu.Controls.Close"),
            std::make_format_args(transformBindingLabel));
        ImGui::BulletText("%s", closeTip.c_str());
        ImGui::PopTextWrapPos();
    }
    SlicedWindow::End();
    if (transformMenuFont) {
        ImGui::PopFont();
    }
    ImGui::PopStyleVar(3);
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

void QueueHUDProcessEvent(MenuEvent event) {
    const SKSE::TaskInterface* taskInterface = SKSE::GetTaskInterface();
    if (!taskInterface) {
        HUD::ProcessEvent(event);
        return;
    }

    taskInterface->AddTask([event]() {
        HUD::ProcessEvent(event);
    });
}

void QueuePickMove(const RE::ObjectRefHandle& handle) {
    const SKSE::TaskInterface* taskInterface = SKSE::GetTaskInterface();
    if (!taskInterface) {
        Picker::MoveEvent(handle);
        return;
    }

    taskInterface->AddTask([handle]() {
        Picker::MoveEvent(handle);
    });
}

void QueuePlacePromptRefresh() {
    const SKSE::TaskInterface* taskInterface = SKSE::GetTaskInterface();
    const auto refresh = []() {
        if (placeSink && clientID && Placer::IsPlacing() &&
            !Menu::IsOpen() && !isTransformMode)
        {
            placeSink->Show();
        }
    };

    if (!taskInterface) {
        refresh();
        return;
    }

    taskInterface->AddTask(refresh);
}

void PlaceSink::ProcessEvent(const SkyPromptAPI::PromptEvent event) {
    if (event.type == SkyPromptAPI::PromptEventType::kTimeout) {
        Hide();
        QueuePlacePromptRefresh();
        return;
    }
    if (event.type == SkyPromptAPI::PromptEventType::kDeclined || event.type == SkyPromptAPI::PromptEventType::kRemovedByMod) {
        Hide();
        QueueHUDProcessEvent(MenuEvent::kPlacerClose);
        return;
    }

    if (event.type == SkyPromptAPI::PromptEventType::kAccepted) {
        switch (event.prompt.eventID) {
            case PLACE_PICK_BUTTON:
                Hide();
                QueueHUDProcessEvent(MenuEvent::kPlacePickAccepted);
                return;
            case PLACE_PLACE_BUTTON:
                Hide();
                QueueHUDProcessEvent(MenuEvent::kPlacePlaceAccepted);
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
            RE::ButtonEvent* button = event->AsButtonEvent();
            if (button && InputConfig::IsActivated(
                    "SkyPrompt.Place.Transform",
                    event)) {
                SetTransformMode(false, false);
                QueuePlacePromptRefresh();
                return true;
            }

            if (button && button->IsDown() &&
                event->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
                const std::uint32_t key = button->GetIDCode();
                if (key == static_cast<std::uint32_t>(
                        RE::BSWin32GamepadDevice::Key::kUp)) {
                    CycleTransformMode(false);
                    return true;
                }
                if (key == static_cast<std::uint32_t>(
                        RE::BSWin32GamepadDevice::Key::kDown)) {
                    CycleTransformMode(true);
                    return true;
                }
            }

            if (button && event->GetDevice() == RE::INPUT_DEVICE::kMouse) {
                if (button->IsDown()) {
                    const std::uint32_t key = button->GetIDCode();
                    if (key == static_cast<std::uint32_t>(
                            RE::BSWin32MouseDevice::Key::kWheelUp)) {
                        CycleTransformMode(false);
                        return true;
                    }
                    if (key == static_cast<std::uint32_t>(
                            RE::BSWin32MouseDevice::Key::kWheelDown)) {
                        CycleTransformMode(true);
                        return true;
                    }
                }
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
            case PICK_MOVE_BUTTON: {
                RE::TESObjectREFR* ref =
                    RE::TESForm::LookupByID<RE::TESObjectREFR>(event.prompt.refid);
                const RE::ObjectRefHandle handle =
                    ref ? ref->GetHandle() : Picker::GetLastHoverHandle();
                if (!handle) {
                    Hide();
                    return;
                }
                Hide();
                QueuePickMove(handle);
                return;
            }
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
