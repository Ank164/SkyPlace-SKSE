#pragma once
#include "Input.h"
#include "Input.h"
#include <unordered_set>
#include <unordered_map>
#include "imgui.h"
struct EventArgs {
    RE::NiPoint2 delta;
};
enum EventType {
    // Single trigger
    kPress,
    kLongPress,
    kDoublePress,
    // Multi trigger
    kHold,
    kHoldMove,
    kAll
};
using ButtonEventHandler = void (*)(EventArgs& args);
class Controls {
    struct Data {
        EventType type;
        std::string name;
        ButtonEventHandler handler;
        bool isDown = false;
        bool wasActiveWhenDown = false;
        bool longPressAccepted = false;
        float pressTime = 0;
    };
    struct Icon {
        Input::Source source;
        Input::Key key;
        Data* data;
    };
    float longPressDuration = 0.004;
    float doublePressTimeout = 0.004;
    RE::ObjectRefHandle attachedRef;
    std::unordered_map<Input::Source, std::unordered_map<uint32_t, int>> sources;
    std::unordered_map<int, Data*> eventData;
    std::unordered_set<int> moveCallbacks;
    bool active = false;
    bool ProcessEvent(RE::InputEvent* event);

    void RenderHint(std::vector<Icon>& icons);
    void RenderHover(std::vector<Icon>& icons);
    void Render();
    void DrawDonut(ImVec2 pos, ImVec2 size, Data* data);

public:
    Controls* AddSink(int eventId, std::string name, EventType type, ButtonEventHandler handler);
    Controls* AddSource(int eventId, Input::Source source, Input::Key key);
    Controls* Show(const RE::ObjectRefHandle& reference = {});
    Controls* Hide();
    void Install();
};

