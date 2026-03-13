#pragma once

namespace PWE::Hooks {
    void* GetEconomy();
    void SetEconomyIfNull(void* economy);
    void ClearEconomy();
}