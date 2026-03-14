#include "DepositHook.hpp"
#include "../bank_economy/Context.hpp"
#include "../bank_misc/TransactionRvaMapping.hpp"
#include "../../PWEOverlay.hpp"
#include "../../Events/Events.hpp"
#include <cstdio>
#include <cstring>
#include <intrin.h>

namespace PWE::Hooks {
    namespace {
        using BankDepositFn = void(__fastcall*)(void* self, void* unknown, int64_t amount, bool flag);

        BankDepositFn o_BankDeposit = nullptr;
        SPF_Hook_Handle* g_bankDepositHook = nullptr;

        int64_t ReadMoneyFromEconomy(void* economy) {
            if (!economy) return 0;
            return *reinterpret_cast<int64_t*>(reinterpret_cast<uintptr_t>(economy) + 0x10);
        }

        void Detour_BankDeposit(void* self, void* unknown, int64_t amount, bool flag) {
            if (self) SetEconomyIfNull(self);

            uintptr_t caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
            uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
            uintptr_t rva = caller - base;

            const char* typeLabel = IdentifyDepositByRva(rva, std::strcmp(g_ctx.gameName, "American Truck Simulator") == 0);
            if (g_ctx.loggerHandle && g_ctx.loadAPI && g_ctx.loadAPI->logger) {
                if(!typeLabel) {
                    char msg[128];
                    std::snprintf(msg, sizeof(msg), "[BankDeposit] Unknown income (RVA: 0x%llX), amount: %lld", (unsigned long long)rva, (long long)amount);
                    g_ctx.loadAPI->logger->LogThrottled(g_ctx.loggerHandle, SPF_LOG_INFO, "PWEOverlay.bank_deposit", 500, msg);
                }
            }

            if(typeLabel) PWE::Events::SendCustomEvent(true, amount, typeLabel, "bank");
            if (o_BankDeposit) o_BankDeposit(self, unknown, amount, flag);
        }
    }  // namespace

    void AddMoney(int64_t amount) {
        void* economy = GetEconomy();
        if (!economy || amount <= 0) return;
        auto* moneyPtr = reinterpret_cast<int64_t*>(reinterpret_cast<uintptr_t>(economy) + 0x10);
        *moneyPtr += amount;
    }

    void RegisterBankDepositHook() {
        if (!g_ctx.coreAPI || !g_ctx.coreAPI->hooks) return;
        if (g_bankDepositHook) return;

        g_bankDepositHook = g_ctx.coreAPI->hooks->Hook_Register(
            PLUGIN_NAME, "BankDeposit", "Bank Deposit Hook", reinterpret_cast<void*>(&Detour_BankDeposit),
            reinterpret_cast<void**>(&o_BankDeposit),
            "57 48 81 EC A0 00 00 00 41 0F B6 F1 49 8B D8 48 8B EA 48 8B F9", true);

        if (g_ctx.loggerHandle && g_ctx.loadAPI && g_ctx.loadAPI->logger) {
            if(!g_bankDepositHook) g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: failed to register BankDeposit hook.");
        }
    }

    void UnregisterBankDepositHook() {
        g_bankDepositHook = nullptr;
        o_BankDeposit = nullptr;
    }
}  // namespace PWE::Hooks