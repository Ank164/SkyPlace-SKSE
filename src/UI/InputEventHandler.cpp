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

RE::InputEvent* InputEventHandler::Process(RE::InputEvent* a_event) {
    RE::InputEvent* first = a_event;
    RE::InputEvent* last = a_event;

    for (RE::InputEvent* current = a_event; current; current = current->next) {
        if (InputEvent(current)) {
            if (current != last) {
                last->next = current->next;
            } else {
                last = current->next;
                first = current->next;
            }
        } else {
            last = current;
        }
    }

    return first;
}


void InputEventHandler::Register(InputEventCallback callback) { callbacks.push_back(callback); }

