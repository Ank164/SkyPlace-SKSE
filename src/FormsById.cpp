#include "FormsById.h"
#include "Config.h"


std::optional<FormsByIdItem> FormsById::Get(RE::TESForm* obj) { 
    if (!obj) {
        return {};
    }
    IF_FIND(items, obj->GetFormID(), it) {
        return it->second;
    }
    return {};
}

void FormsById::Install() {
    Config::Each("Data/", "_Item_SP.json", [](std::string file, json data) {
        logger::info("Loading item file: {}", file);
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

            IF_NOT_GET_JSON_STRING(item, objectEditorId, "ObjectEditorId") {
                logger::error("Missing required field ObjectEditorId");
                continue;
            }

            IF_NOT_GET_JSON_FLOAT(item, weight, "Weight") {
                logger::error("config: no Weight");
            }

            IF_NOT_GET_JSON_INT(item, value, "Value") {
                logger::error("config: no Value");
            }
            IF_NOT_GET_JSON_STRING(item, name, "Name") {
                logger::error("config: no Name");
            }

            auto obj = RE::TESForm::LookupByEditorID(objectEditorId);

            if (!obj) {
                logger::error("Object not found");
                continue;
            }
            FormID objectId = obj->GetFormID();

            FormsByIdItem result = {};

            result.objectId = objectId;

            if (weight != -1) {
                result.weight = weight;
            }
            if (value != -1) {
                result.value = value;
            }
            if (name.size() > 0) {
                result.name = name;
            }

            items[objectId] = result;
        }
    });
}
