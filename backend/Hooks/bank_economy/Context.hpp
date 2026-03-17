#pragma once

namespace PWE::Hooks::Economy::Internal {
    void* GetEconomy();
    void SetEconomyIfNull(void* economy);
    void ClearEconomy();
}