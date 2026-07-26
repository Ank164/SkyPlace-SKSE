#include "SkyPlaceConfig.h"

#include <fstream>

#include "Config.h"

namespace {
    constexpr const char* configPath = "Data/SKSE/Plugins/SkyPlaceConfig.json";
    constexpr const char* disableSoulGemCloningKey = "DisableSoulGemCloning";
    constexpr const char* moveEverythingIgnoringFiltersKey =
        "MoveEverythingIgnoringFilters";

    bool isSoulGemCloningDisabled = true;
    bool shouldIgnoreFilters = false;

    void Save() {
        const json data = {
            {disableSoulGemCloningKey, isSoulGemCloningDisabled},
            {moveEverythingIgnoringFiltersKey, shouldIgnoreFilters}
        };

        std::ofstream file(configPath, std::ios::trunc);
        if (!file) {
            logger::error("Failed to open SkyPlace config for writing: {}", configPath);
            return;
        }

        file << data.dump(2) << '\n';
        if (!file) {
            logger::error("Failed to write SkyPlace config: {}", configPath);
        }
    }

    void LoadBooleanValue(
        const json& data,
        const char* key,
        bool& value)
    {
        const json::const_iterator entry = data.find(key);
        if (entry == data.end()) {
            logger::warn("Missing SkyPlace config setting '{}'; using default", key);
            return;
        }
        if (!entry->is_boolean()) {
            logger::warn("SkyPlace config setting '{}' must be a boolean; using default", key);
            return;
        }

        value = entry->get<bool>();
    }
}

void SkyPlaceConfig::Load() {
    try {
        const json data = Config::Get(configPath);
        if (!data.is_object()) {
            logger::error("SkyPlace config must contain a JSON object: {}", configPath);
            return;
        }

        LoadBooleanValue(
            data,
            disableSoulGemCloningKey,
            isSoulGemCloningDisabled);
        LoadBooleanValue(
            data,
            moveEverythingIgnoringFiltersKey,
            shouldIgnoreFilters);
    } catch (const Config::FailedToOpenFileError&) {
        logger::info("SkyPlace config not found; creating defaults: {}", configPath);
        Save();
    } catch (const json::exception& error) {
        logger::error("Failed to parse SkyPlace config '{}': {}", configPath, error.what());
    }
}

bool SkyPlaceConfig::IsSoulGemCloningDisabled() {
    return isSoulGemCloningDisabled;
}

void SkyPlaceConfig::SetSoulGemCloningDisabled(bool value) {
    if (isSoulGemCloningDisabled == value) {
        return;
    }

    isSoulGemCloningDisabled = value;
    Save();
}

bool SkyPlaceConfig::ShouldIgnoreFilters() {
    return shouldIgnoreFilters;
}

void SkyPlaceConfig::SetIgnoreFilters(bool value) {
    if (shouldIgnoreFilters == value) {
        return;
    }

    shouldIgnoreFilters = value;
    Save();
}
