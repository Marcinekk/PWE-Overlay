#pragma once
#include <cstdint>

namespace PWE::Hooks::Economy {
    void RegisterDeposit();
    void UnregisterDeposit();
    void AddMoney(int64_t amount);
}