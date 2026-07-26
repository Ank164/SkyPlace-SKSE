#include "InputEventHandler.h"
bool InputEventHandler::InputEvent(RE::InputEvent* event) {

    bool result = false;

    for (auto item : callbacks) {
        if (item(event)) {
            result = true;
        }
    }

    return result;
}

RE::InputEvent* const* InputEventHandler::Process(RE::InputEvent* const* a_event) { 

    auto first = *a_event;
    auto last = *a_event;
    size_t length = 0;

    for (auto current = *a_event; current; current = current->next) {
        if (InputEvent(current)) {
            if (current != last) {
                last->next = current->next;
            } else {
                last = current->next;
                first = current->next;
            }
        } else {
            last = current;
            ++length;
        }
    }

    RE::InputEvent* const e[] = {first};
    return e;
}


void InputEventHandler::Register(InputEventCallback callback) { callbacks.push_back(callback); }

