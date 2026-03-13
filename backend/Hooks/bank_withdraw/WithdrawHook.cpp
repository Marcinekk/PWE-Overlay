#include "WithdrawHook.hpp"
#include "../bank_economy/Context.hpp"
#include "../bank_misc/TransactionRvaMapping.hpp"
#include "../../PWEOverlay.hpp"
#include "../../Events/Events.hpp"
#include <cstdio>
#include <intrin.h>

namespace PWE::Hooks {
    namespace {
        using BankWithdrawFn = void(__fastcall*)(void* self, void* unknown, int64_t amount, bool flag);

        BankWithdrawFn o_BankWithdraw = nullptr;
        SPF_Hook_Handle* g_bankWithdrawHook = nullptr;

        int64_t ReadMoneyFromEconomy(void* economy) {
            if (!economy) return 0;
            return *reinterpret_cast<int64_t*>(reinterpret_cast<uintptr_t>(economy) + 0x10);
        }

        void Detour_BankWithdraw(void* self, void* unknown, int64_t amount, bool flag) {
            if (self) SetEconomyIfNull(self);

            uintptr_t caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
            uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
            uintptr_t rva = caller - base;

            const char* typeLabel = IdentifyWithdrawByRva(rva);
            if (g_ctx.loggerHandle && g_ctx.loadAPI && g_ctx.loadAPI->logger) {
                if(!typeLabel) {
                    char msg[128];
                    std::snprintf(msg, sizeof(msg), "[BankWithdraw] Unknown Expense (RVA: 0x%llX), amount: %lld", (unsigned long long)rva, (long long)amount);
                    g_ctx.loadAPI->logger->LogThrottled(g_ctx.loggerHandle, SPF_LOG_INFO, "PWEOverlay.bank_withdraw", 500, msg);
                }
            }

            if (typeLabel) PWE::Events::SendCustomEvent(false, amount, typeLabel, "bank");
            if (o_BankWithdraw) o_BankWithdraw(self, unknown, amount, flag);
        }
    }  // namespace

    int64_t GetMoney() {
        void* economy = GetEconomy();
        if (!economy) return 0;
        return *reinterpret_cast<int64_t*>(reinterpret_cast<uintptr_t>(economy) + 0x10);
    }

    void RemoveMoney(int64_t amount) {
        void* economy = GetEconomy();
        if (!economy || amount <= 0) return;
        auto* moneyPtr = reinterpret_cast<int64_t*>(reinterpret_cast<uintptr_t>(economy) + 0x10);
        if (*moneyPtr <= amount) *moneyPtr = 0;
        else *moneyPtr -= amount;
    }

    void RegisterBankWithdrawHook() {
        if (!g_ctx.coreAPI || !g_ctx.coreAPI->hooks) return;
        if (g_bankWithdrawHook) return;

        g_bankWithdrawHook = g_ctx.coreAPI->hooks->Hook_Register(
            PLUGIN_NAME, "BankWithdraw", "Bank Withdraw Hook", reinterpret_cast<void*>(&Detour_BankWithdraw),
            reinterpret_cast<void**>(&o_BankWithdraw),
            "57 48 81 EC A0 00 00 00 41 0F B6 F1 49 8B D8 48 8B EA 48 8B F9 4D 85 C0", true);

        if (g_ctx.loggerHandle && g_ctx.loadAPI && g_ctx.loadAPI->logger) {
            if(!g_bankWithdrawHook) g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: failed to register BankWithdraw hook.");
        }
    }

    void UnregisterBankWithdrawHook() {
        g_bankWithdrawHook = nullptr;
        o_BankWithdraw = nullptr;
    }
}  // namespace PWE::Hooks