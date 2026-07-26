#pragma once

namespace StringUtil {
    bool StartsWith(std::string target, std::string prefix);
    std::string Normalize(const std::string& path);
    std::string JoinPath(const std::vector<std::string> paths);
}