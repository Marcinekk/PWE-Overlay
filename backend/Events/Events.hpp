#pragma once

#include <cstdint>

namespace PWE::Events {
    void SendCustomEvent(bool income, int64_t amount, const char* description, const char* type);
    void RegisterEvents();
    void UnregisterEvents();
}