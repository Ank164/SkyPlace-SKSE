#pragma once


struct LogItem {
    uint32_t time;
    std::string message;
};
class ScreenLog {
    static inline std::vector<LogItem> Logs;
    static void Render();
public:
    static void Install();
    static void Add(std::string message);
    template <typename... Args>
    static void Add(std::format_string<Args...> fmt, Args&&... args) {
        Add(std::format(fmt, std::forward<Args>(args)...));
    }
};