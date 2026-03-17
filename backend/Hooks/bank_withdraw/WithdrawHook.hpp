#pragma once
#include <cstdint>

namespace PWE::Hooks::Economy {
    void RegisterWithdraw();
    void UnregisterWithdraw();
    void RemoveMoney(int64_t amount);
    int64_t GetMoney();
}