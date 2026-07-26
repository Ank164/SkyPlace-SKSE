#include "StringUtil.h"

bool StringUtil::StartsWith(std::string target, std::string prefix) {
    if (prefix.size() > target.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (target[i] != prefix[i]) return false;
    }
    return true;
}

std::string StringUtil::Normalize(const std::string& path) {
    std::string result = path;
    for (char& ch : result) {
        if (ch == '\\' || ch == '/') {
            ch = '/';
        }
    }
    return result;
}

std::string StringUtil::JoinPath(const std::vector<std::string> paths) {
    std::string result;
    for (const auto& path : paths) {
        if (!result.empty()) {
            result += "/";
        }
        result += path;
    }
    return result;
}