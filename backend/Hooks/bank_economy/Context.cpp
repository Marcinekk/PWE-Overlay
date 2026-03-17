#include "Context.hpp"

#include <cstdint>

namespace PWE::Hooks::Economy::Internal {
    namespace {
        void* g_economy = nullptr;

        bool IsLikelyValidPointer(void* p) {
            const uintptr_t value = reinterpret_cast<uintptr_t>(p);

            if (value < 0x100000000ULL) return false;
            if ((value & 0x7ULL) != 0) return false;
            return true;
        }
    }

    void* GetEconomy() {
        return g_economy;
    }

    void SetEconomyIfNull(void* economy) {
        if (!g_economy && IsLikelyValidPointer(economy)) g_economy = economy;
    }

    void ClearEconomy() {
        g_economy = nullptr;
    }
}  // namespace PWE::Hooks