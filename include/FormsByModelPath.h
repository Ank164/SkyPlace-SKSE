#pragma once

#include <regex>

struct FormsByModelPathItem {
    std::string pattern;
    std::regex regex;
    std::string name;
    float weight;
    int32_t value;
};

struct FormsByModelPathBlacklistItem {
    std::string pattern;
    std::regex regex;
};

class FormsByModelPath {
    static inline std::vector<FormsByModelPathItem> items;
    static inline std::vector<FormsByModelPathBlacklistItem> blacklist;
    static inline std::vector<FormsByModelPathBlacklistItem> blacklistNoExceptions;
    static inline std::vector<FormsByModelPathBlacklistItem> blacklistExceptions;
    static void Process();
public:
    static std::optional<FormsByModelPathItem> Get(RE::TESForm* obj);
    static void Install();
};
