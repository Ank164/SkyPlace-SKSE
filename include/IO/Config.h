#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace Config {
    inline const char* dataFolder = "Data/";
    inline const char* pluginFolder = "Data/SKSE/Plugins";
    class FailedToOpenFileError {};
    void Each(std::string directory, std::string pattern, const std::function<void(std::string, json)>& callback);
    json Get(std::string path);
}

#define IF_NOT_GET_JSON_STRING(json, variable, name) \
    std::string variable = "";                            \
    if (!(json).contains(name) || !(json)[name].is_string() || !((variable = (json)[name]), true))

#define IF_NOT_GET_JSON_FLOAT(json, variable, name) \
    float variable = -1;                            \
    if (!(json).contains(name) || !(json)[name].is_number() || !((variable = (json)[name]), true))

#define IF_NOT_GET_JSON_INT(json, variable, name) \
    int variable = -1;                                 \
    if (!(json).contains(name) || !(json)[name].is_number() || !((variable = (json)[name]), true))

#define IF_GET_JSON_OBJ(obj, variable, name) \
    json variable;                           \
    if ((obj).contains(name) && (obj)[name].is_object() && ((variable = (obj)[name]), true))




#define IF_GET_JSON_FLOAT(obj, variable, name) \
    float variable;                           \
    if ((obj).contains(name) && (obj)[name].is_number() && ((variable = (obj)[name]), true))
