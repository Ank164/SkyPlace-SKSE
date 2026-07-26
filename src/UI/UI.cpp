#include "UI.h"

#include "SKSEMenuFramework.h"
#include "Controls.h"
#include "HUD.h"
#include "InputConfig.h"
#include "Menu.h"
#include "Translations.h"
#include "Hooks.h"
#include "ObjectGroup.h"
#include "Picker.h"
#include "Shader.h"
#include "SkyPlaceConfig.h"

namespace {
    bool isMenuFrameworkOpen = false;

    void __stdcall OnMenuEvent(SKSEMenuFramework::Model::EventType eventType) {
        switch (eventType) {
            case SKSEMenuFramework::Model::EventType::kOpenMenu:
                isMenuFrameworkOpen = true;
                HUD::OnMenuOpen();
                break;
            case SKSEMenuFramework::Model::EventType::kCloseMenu:
                isMenuFrameworkOpen = false;
                HUD::OnMenuClose();
                break;
            default:
                break;
        }
    }

    bool __stdcall OnInput(RE::InputEvent* event) {
        if (!event ||
            isMenuFrameworkOpen ||
            SKSEMenuFramework::IsAnyBlockingWindowOpened() ||
            Menu::IsOpen()) {
            return false;
        }

        if (InputConfig::IsActivated("SkyPrompt.EnableDisable", event)) {
            HUD::SetIsEnabled(!HUD::GetIsEnabled());
            return true;
        }

        return false;
    }

    void RenderFormID(RE::FormID formID, std::string_view state) {
        const std::string label = std::format(
            "0x{:08X} | {} | {}",
            formID,
            state,
            RE::TESForm::LookupByID(formID) ? "loaded" : "missing");
        ImGuiMCP::BulletText("%s", label.c_str());
    }

}

void UI::Register() {
    if (!SKSEMenuFramework::IsInstalled()) {
        return;
    }
    SKSEMenuFramework::SetSection(BEAUTIFUL_NAME);
    SKSEMenuFramework::AddSectionItem(
        Translations::Get("Main.Section"),
        MainWindow::Render);
    SKSEMenuFramework::AddSectionItem(
        Translations::Get("Config.Section"),
        ConfigWindow::Render);
    SKSEMenuFramework::AddSectionItem(
        Translations::Get("Health.DynamicForms.Section"),
        HealthWindow::RenderDynamicForms);
    SKSEMenuFramework::AddSectionItem(
        Translations::Get("Health.RetainedResources.Section"),
        HealthWindow::RenderRetainedResources);
    SKSEMenuFramework::AddEvent(OnMenuEvent, 0.0f);
    SKSEMenuFramework::AddInputEvent(OnInput);
}
void __stdcall UI::MainWindow::Render() {
    bool isEnabled = HUD::GetIsEnabled();
    const std::string bindingLabel = InputConfig::GetBindingLabel("SkyPrompt.EnableDisable");
    const char* toggleText = Translations::Get(
        isEnabled ? "SkyPrompt.Toggle.Exit" : "SkyPrompt.Toggle.Enter");
    const std::string toggleLabel = bindingLabel.empty() ?
        std::format("{}##SkyPlaceEnabled", toggleText) :
        std::format("{} ({})##SkyPlaceEnabled", toggleText, bindingLabel);

    if (ImGuiMCPComponents::ToggleButton(toggleLabel.c_str(), &isEnabled)) {
        HUD::SetIsEnabled(isEnabled);
    }
}

void __stdcall UI::ConfigWindow::Render() {
    bool isSoulGemCloningDisabled =
        SkyPlaceConfig::IsSoulGemCloningDisabled();
    const std::string soulGemCloningLabel = std::format(
        "{}##SkyPlaceSoulGemCloning",
        Translations::Get("Config.DisableSoulGemCloning"));
    if (ImGuiMCPComponents::ToggleButton(
            soulGemCloningLabel.c_str(),
            &isSoulGemCloningDisabled))
    {
        SkyPlaceConfig::SetSoulGemCloningDisabled(isSoulGemCloningDisabled);
        if (isSoulGemCloningDisabled) {
            HUD::HideInventoryClone();
        }
    }

    bool shouldIgnoreFilters = SkyPlaceConfig::ShouldIgnoreFilters();
    const std::string ignoreFiltersLabel = std::format(
        "{}##SkyPlaceIgnoreFilters",
        Translations::Get("Config.IgnoreFilters"));
    if (ImGuiMCPComponents::ToggleButton(
            ignoreFiltersLabel.c_str(),
            &shouldIgnoreFilters))
    {
        SkyPlaceConfig::SetIgnoreFilters(shouldIgnoreFilters);
        Picker::SaveChangeEvent();
        Hooks::RefreshLoadedReferenceHighlights();
    }
}

void __stdcall UI::HealthWindow::RenderDynamicForms() {
    const std::map<RE::FormID, ObjectGroup::Data>& groups = ObjectGroup::GetAll();
    const std::vector<RE::FormID>& retiredItems = ObjectGroup::GetRetiredItems();
    std::size_t dynamicGroupCount = 0;
    std::size_t dynamicRetiredCount = 0;
    std::size_t storedInventoryChestCount = 0;

    for (const auto& groupEntry : groups) {
        if (IsDynamicId(groupEntry.first)) {
            ++dynamicGroupCount;
        }
    }
    for (const RE::FormID formID : retiredItems) {
        if (IsDynamicId(formID)) {
            ++dynamicRetiredCount;
        }
    }
    for (const auto& [itemFormID, group] : groups) {
        for (const ObjectGroup::Member& member : group.members) {
            if (member.inventoryChestRefID != 0) {
                ++storedInventoryChestCount;
            }
        }
    }

    ImGuiMCP::Text(
        "SkyPlace tracked dynamic forms: %zu",
        dynamicGroupCount + dynamicRetiredCount);
    ImGuiMCP::Text("Active group forms: %zu", dynamicGroupCount);
    ImGuiMCP::Text("Retired forms kept alive: %zu", dynamicRetiredCount);
    ImGuiMCP::Text("Stored inventory chests: %zu", storedInventoryChestCount);

    ImGuiMCP::SeparatorText("Active dynamic group forms");
    for (const auto& [itemFormID, group] : groups) {
        if (IsDynamicId(itemFormID)) {
            ImGuiMCP::BulletText(
                "0x%08X | members: %zu | mesh: %s | %s",
                itemFormID,
                group.members.size(),
                group.meshPath.c_str(),
                RE::TESForm::LookupByID(itemFormID) ? "loaded" : "missing");
        }
    }

    ImGuiMCP::SeparatorText("Retired dynamic group forms");
    for (const RE::FormID formID : retiredItems) {
        if (IsDynamicId(formID)) {
            RenderFormID(formID, "retained until session end");
        }
    }

    ImGuiMCP::SeparatorText("Stored inventory chests");
    for (const auto& [itemFormID, group] : groups) {
        for (std::size_t memberIndex = 0; memberIndex < group.members.size(); ++memberIndex) {
            const ObjectGroup::Member& member = group.members[memberIndex];
            if (member.inventoryChestRefID == 0) {
                continue;
            }

            ImGuiMCP::BulletText(
                "chest 0x%08X <- group 0x%08X member %zu object 0x%08X | %s",
                member.inventoryChestRefID,
                itemFormID,
                memberIndex,
                member.objectFormID,
                RE::TESForm::LookupByID(member.inventoryChestRefID) ? "loaded" : "missing");
        }
    }
}

void __stdcall UI::HealthWindow::RenderRetainedResources() {
    const std::unordered_map<RE::FormID, RE::ObjectRefHandle> highlightedReferences =
        Shader::GetHighlightedReferences();
    std::size_t orphanedGroupModelCount = 0;
    std::size_t staleReferenceHandleCount = 0;

    ImGuiMCP::SeparatorText("Owned group models");
    ImGuiMCP::Text("Owned raw model entries: %zu", Hooks::GetGroupModelCacheSize());
    Hooks::VisitGroupModelCache([&orphanedGroupModelCount](std::string_view meshPath) {
        const bool hasGroup = ObjectGroup::GetByMesh(meshPath) != nullptr;
        if (!hasGroup) {
            ++orphanedGroupModelCount;
        }
        ImGuiMCP::BulletText(
            "%s | %s",
            meshPath.data(),
            hasGroup ? "active group" : "orphaned");
    });
    ImGuiMCP::Text("Orphaned model entries: %zu", orphanedGroupModelCount);

    ImGuiMCP::SeparatorText("Retained reference handles");
    ImGuiMCP::Text("Tracked highlighted references: %zu", highlightedReferences.size());
    for (const auto& [formID, handle] : highlightedReferences) {
        const bool resolved = handle.get() != nullptr;
        if (!resolved) {
            ++staleReferenceHandleCount;
        }
        ImGuiMCP::BulletText(
            "0x%08X | %s",
            formID,
            resolved ? "resolved" : "stale handle");
    }
    ImGuiMCP::Text("Stale reference handles: %zu", staleReferenceHandleCount);
}
