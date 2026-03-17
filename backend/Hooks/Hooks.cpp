#include "Hooks.hpp"

#include "bank_deposit/DepositHook.hpp"
#include "bank_withdraw/WithdrawHook.hpp"
#include "bank_economy/BootstrapHook.hpp"
#include "bank_economy/Context.hpp"


namespace PWE::Hooks {
    void RegisterAllHooks() {
        Economy::Internal::Register();
        Economy::RegisterDeposit();
        Economy::RegisterWithdraw();
    }

    void UnregisterAllHooks() {
        Economy::UnregisterDeposit();
        Economy::UnregisterWithdraw();
        Economy::Internal::Unregister();
        Economy::Internal::ClearEconomy();
    }
}  // namespace PWE::Hooks