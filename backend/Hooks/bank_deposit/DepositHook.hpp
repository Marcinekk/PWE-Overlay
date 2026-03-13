#pragma once
#include <cstdint>

namespace PWE::Hooks {
    void RegisterBankDepositHook();
    void UnregisterBankDepositHook();
    void AddMoney(int64_t amount);
}