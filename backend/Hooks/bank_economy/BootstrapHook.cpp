#include "BootstrapHook.hpp"

#include <cstdio>
#include <cstdint>

#include "../../PWEOverlay.hpp"
#include "Context.hpp"

namespace PWE::Hooks {
    namespace {
        using EconomyCreateFn = void*(__fastcall*)(void* owner);
        using EconomyGetterFn = void*(__fastcall*)(void* self);

        EconomyCreateFn o_EconomyCreate = nullptr;
        EconomyGetterFn o_EconomyGetter = nullptr;
        SPF_Hook_Handle* g_economyBootstrapHook = nullptr;
        SPF_Hook_Handle* g_economyGetterHook = nullptr;
        bool g_loggedBootstrapCapture = false;

        bool IsLikelyValidPointer(void* p) {
            const uintptr_t value = reinterpret_cast<uintptr_t>(p);
            return value >= 0x100000000ULL && (value & 0x7ULL) == 0;
        }

        void TryCaptureEconomy(const char* source, void* economy, void* ownerOrSelf) {
            if (!IsLikelyValidPointer(economy)) return;
            SetEconomyIfNull(economy);
            if (!g_loggedBootstrapCapture && g_ctx.loggerHandle && g_ctx.loadAPI && g_ctx.loadAPI->logger) g_loggedBootstrapCapture = true;
        }

        void* Detour_EconomyCreate(void* owner) {
            void* economy = o_EconomyCreate ? o_EconomyCreate(owner) : nullptr;
            TryCaptureEconomy("create", economy, owner);
            return economy;
        }

        void* Detour_EconomyGetter(void* self) {
            void* returned = o_EconomyGetter ? o_EconomyGetter(self) : nullptr;

            void* economy = nullptr;
            if (IsLikelyValidPointer(self)) {
                auto* level0 = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(self) + 0xC8);
                if (IsLikelyValidPointer(level0)) economy = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(level0) + 0x10);
            }

            TryCaptureEconomy("getter", economy, self);
            return returned;
        }
    }  // namespace

    void RegisterEconomyBootstrapHook() {
        if (!g_ctx.coreAPI || !g_ctx.coreAPI->hooks) return;
        if (g_economyBootstrapHook && g_economyGetterHook) return;

        if (!g_economyBootstrapHook) {
            g_economyBootstrapHook = g_ctx.coreAPI->hooks->Hook_Register(
                PLUGIN_NAME, "EconomyCreate_Bootstrap", "Economy Create Bootstrap",
                reinterpret_cast<void*>(&Detour_EconomyCreate),
                reinterpret_cast<void**>(&o_EconomyCreate),
                "48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 8B 41 08 48 8B F9 25 00 00 00 06", true);
        }

        if (!g_economyGetterHook) {
            g_economyGetterHook = g_ctx.coreAPI->hooks->Hook_Register(
                PLUGIN_NAME, "EconomyGetter_Bootstrap", "Economy Getter Bootstrap",
                reinterpret_cast<void*>(&Detour_EconomyGetter),
                reinterpret_cast<void**>(&o_EconomyGetter),
                "48 8B 81 C8 00 00 00 48 85 C0 74 09 48 8B 40 10 48 8B 40 10", true);
        }

        if (g_ctx.loggerHandle && g_ctx.loadAPI && g_ctx.loadAPI->logger) {
            if(!g_economyBootstrapHook && !g_economyGetterHook) g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: failed to register Economy bootstrap hooks.");
        }
    }

    void UnregisterEconomyBootstrapHook() {
        g_economyBootstrapHook = nullptr;
        g_economyGetterHook = nullptr;
        o_EconomyCreate = nullptr;
        o_EconomyGetter = nullptr;
        g_loggedBootstrapCapture = false;
    }
}  // namespace PWE::Hooks