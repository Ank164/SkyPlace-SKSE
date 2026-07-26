#pragma once

namespace SkyPlaceConfig {
    void Load();

    bool IsSoulGemCloningDisabled();
    void SetSoulGemCloningDisabled(bool value);

    bool ShouldIgnoreFilters();
    void SetIgnoreFilters(bool value);
}
