#pragma once

#include <spdlog/sinks/basic_file_sink.h>

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

namespace logger = SKSE::log;
using namespace std::literals;

using FormID = RE::FormID;
using RefID = RE::FormID;

const RefID player_refid = 20;

inline bool IsDynamicId(FormID id) { return id >= 0xFF000000; }

using KeyboardKeys = RE::BSWin32KeyboardDevice::Key;
using GamepadKeys = RE::BSWin32GamepadDevice::Key;
using GamepadOrbisKeys = RE::BSPCOrbisGamepadDevice::Key;
using MouseButtons = RE::BSWin32MouseDevice::Key;
enum Move { kMouseMove = 283, kThumbstickMove = 284 };

#define IF_FIND(array, value, it) \
    auto it = array.find(value);  \
    if (it != array.end())
