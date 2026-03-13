#pragma once
#include <cstdint>

namespace PWE::Hooks {
    void RegisterBankWithdrawHook();
    void UnregisterBankWithdrawHook();
    void RemoveMoney(int64_t amount);
    int64_t GetMoney();
}