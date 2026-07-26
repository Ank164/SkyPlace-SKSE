#include "Config.h"

void Config::Each(std::string directory, std::string pattern, const std::function<void(std::string, json)>& callback) {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            const std::string filename = entry.path().filename().string();

            if (filename.size() >= pattern.size() && filename.substr(filename.size() - pattern.size()) == pattern) {
                std::ifstream f(entry.path());
                if (!f) {
                    continue;
                }

                try {
                    const json data = json::parse(f);

                    callback(filename, data);

                } catch (const json::parse_error&) {  // NOLINT(bugprone-empty-catch)
                } 
                
                f.close();
            }
        }
    }
}

json Config::Get(std::string path) { 
    std::ifstream f(path);
    if (!f) {
        throw FailedToOpenFileError();
    }
    json data = json::parse(f);
    f.close();
    return data;
}
