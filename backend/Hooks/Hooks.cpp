#include "Hooks.hpp"

#include "bank_deposit/DepositHook.hpp"
#include "bank_withdraw/WithdrawHook.hpp"
#include "bank_economy/BootstrapHook.hpp"
#include "bank_economy/Context.hpp"

namespace PWE::Hooks {
    void RegisterAllHooks() {
        RegisterEconomyBootstrapHook();
        RegisterBankDepositHook();
        RegisterBankWithdrawHook();
    }

    void UnregisterAllHooks() {
        UnregisterEconomyBootstrapHook();
        UnregisterBankDepositHook();
        UnregisterBankWithdrawHook();
        ClearEconomy();
    }
}  // namespace PWE::Hooks