#include "ScreenLog.h"
#include "Graphics.h"

void ScreenLog::Install() {
	Graphics::Register(Render); }

void ScreenLog::Add(std::string message) {
    auto now = RE::GetDurationOfApplicationRunTime(); 
	Logs.push_back(LogItem{now, message});
    while (Logs.size() > 50) Logs.erase(Logs.begin());
}


void ScreenLog::Render() {
    auto fg = ImGui::GetForegroundDrawList();
    float y = 10;
    for (auto it = Logs.rbegin(); it != Logs.rend(); ++it) {
        y += 30;
        fg->AddText({50+1, y+1}, ImColor(0, 0, 0, 255), it->message.c_str());
        fg->AddText({50, y}, ImColor(255, 255, 255, 255), it->message.c_str());
    }
}