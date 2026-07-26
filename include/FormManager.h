#pragma once

struct FormManagerData {
    std::string name;
    float weight = 0;
    int32_t value = 0;
};

class FormManager {
public:
    static std::optional<FormManagerData> Get(RE::FormID form);
};
