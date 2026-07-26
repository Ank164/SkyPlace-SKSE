#include "FormsByModelPath.h"
#include "Config.h"
#include "unordered_set"
#include "StringUtil.h"
//inline RE::TESForm* GetForm(const char* fileName, RE::FormID localId) {
//    const auto dataHandler = RE::TESDataHandler::GetSingleton();
//    auto formId = dataHandler->LookupFormID(localId, fileName);
//    return RE::TESForm::LookupByID(formId);
//}



std::optional<FormsByModelPathItem> FormsByModelPath::Get(RE::TESForm* obj) {

    if (!obj) {
        return {};
    }

    const char* path = nullptr;
    auto model = obj->As<RE::TESModel>();
    auto armo = obj->As<RE::TESObjectARMO>();
    if (armo) {
        path = armo->worldModels[0].GetModel();
    } else if (model) {
        path = model->GetModel();
    } else {
        return {};
    }

    auto cleanModel = StringUtil::Normalize(path);

    if (!obj->IsInventoryObject()) {
        for (const auto& item : blacklistNoExceptions) {
            if (std::regex_search(cleanModel, item.regex)) {
                return {};
            }
        }

        bool isBlacklistException = false;
        for (const auto& item : blacklistExceptions) {
            if (std::regex_search(cleanModel, item.regex)) {
                isBlacklistException = true;
                break;
            }
        }

        if (!isBlacklistException) {
            for (const auto& item : blacklist) {
                if (std::regex_search(cleanModel, item.regex)) {
                    return {};
                }
            }
        }
    }

    for (const auto& item : items) {
        if (std::regex_search(cleanModel, item.regex)) {
            return item;
        }
    }

    return {};
}

void FormsByModelPath::Install() {
    Config::Each("Data/", "_FilterBlacklistException_SP.json", [](std::string file, json data) {
        logger::info("Loading blacklist exception file: {}", file);
        if (!data.is_array()) {
            logger::error("the file must be a json array");
            return;
        }
        size_t i = 0;
        for (auto item : data) {
            logger::info("Reading index: {}", i);
            ++i;
            if (!item.is_string()) {
                logger::error("item must be string");
                continue;
            }
            const auto pattern = item.get<std::string>();
            try {
                blacklistExceptions.push_back({
                    pattern,
                    std::regex(pattern, std::regex::ECMAScript)
                });
            } catch (const std::regex_error& error) {
                logger::error("Invalid blacklist exception regex '{}': {}", pattern, error.what());
            }
        }
    });
    Config::Each("Data/", "_FilterBlacklistNoException_SP.json", [](std::string file, json data) {
        logger::info("Loading blacklist without exceptions file: {}", file);
        if (!data.is_array()) {
            logger::error("the file must be a json array");
            return;
        }
        size_t i = 0;
        for (auto item : data) {
            logger::info("Reading index: {}", i);
            ++i;
            if (!item.is_string()) {
                logger::error("item must be string");
                continue;
            }
            const auto pattern = item.get<std::string>();
            try {
                blacklistNoExceptions.push_back({
                    pattern,
                    std::regex(pattern, std::regex::ECMAScript)
                });
            } catch (const std::regex_error& error) {
                logger::error("Invalid blacklist without exceptions regex '{}': {}", pattern, error.what());
            }
        }
    });
    Config::Each("Data/", "_FilterBlacklist_SP.json", [](std::string file, json data) {
        logger::info("Loading blacklist file: {}", file);
        if (!data.is_array()) {
            logger::error("the file must be a json array");
            return;
        }
        size_t i = 0;
        for (auto item : data) {
            logger::info("Reading index: {}", i);
            ++i;
            if (!item.is_string()) {
                logger::error("item must be string");
                continue;
            }
            const auto pattern = item.get<std::string>();
            try {
                blacklist.push_back({
                    pattern,
                    std::regex(pattern, std::regex::ECMAScript)
                });
            } catch (const std::regex_error& error) {
                logger::error("Invalid blacklist regex '{}': {}", pattern, error.what());
            }
        }
    });
    Config::Each("Data/", "_Filter_SP.json", [](std::string file, json data) {
        logger::info("Loading filter file: {}", file);
        if (!data.is_array()) {
            logger::error("the file must be a json array");
            return;
        }
        size_t i = 0;
        for (auto item : data) {
            logger::info("Reading index: {}", i);
            ++i;

            if (!item.is_object()) {
                logger::error("Item must be an object");
                continue;
            }

            IF_NOT_GET_JSON_STRING(item, pattern, "Regex") {
                logger::error("Missing Regex");
                continue;
            }

            IF_NOT_GET_JSON_FLOAT(item, weight, "Weight") {
                logger::error("Missing Weight");
                continue;
            }

            IF_NOT_GET_JSON_INT(item, value, "Value") {
                logger::error("Missing Value");
                continue;
            }
            IF_NOT_GET_JSON_STRING(item, name, "Name") {
                logger::error("Missing Name");
                continue;
            }
            try {
                items.push_back({
                    pattern,
                    std::regex(pattern, std::regex::ECMAScript),
                    name,
                    weight,
                    value
                });
            } catch (const std::regex_error& error) {
                logger::error("Invalid filter regex '{}': {}", pattern, error.what());
            }
        }
    });

    Process();

    for (const auto& item : items) {
        logger::trace("config: {} {} {}", item.pattern, item.weight, item.value);
    }
}

void FormsByModelPath::Process() {
    std::sort(items.begin(), items.end(), [](const FormsByModelPathItem& a, const FormsByModelPathItem& b) {
        return a.pattern.length() > b.pattern.length();
    });
    std::unordered_set<std::string> seen;
    items.erase(std::remove_if(items.begin(), items.end(), [&](const FormsByModelPathItem& i) {
        return !seen.insert(i.pattern).second;
    }), items.end());
}
