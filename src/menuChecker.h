// Shizof's method
#pragma once

#include "mod_eventSink.hpp"
#include <unordered_map>

namespace MenuChecker
{
    extern std::vector<std::string> gameStoppingMenus;

    extern std::unordered_map<std::string, bool> menuTypes;

    bool isGameStopped();

    void begin();

    void onMenuOpenClose(RE::MenuOpenCloseEvent const *evn);
}
