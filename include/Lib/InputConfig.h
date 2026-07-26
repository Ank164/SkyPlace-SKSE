#pragma once

namespace InputConfig {
    void Install();
    std::vector<std::pair<RE::INPUT_DEVICE, uint32_t>> Get(const std::string& key);
    bool IsActivated(const std::string& key, RE::InputEvent* event);
    std::string GetBindingLabel(const std::string& key);
}
