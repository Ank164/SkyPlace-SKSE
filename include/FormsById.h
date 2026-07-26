#pragma once

struct FormsByIdItem {
    RE::FormID objectId;
    std::optional<std::string> name;
    std::optional<float> weight;
    std::optional<int> value;
};

class FormsById {
    static inline std::map<FormID, FormsByIdItem> items;

public:
    static std::optional<FormsByIdItem> Get(RE::TESForm* obj);
    static void Install();
};
