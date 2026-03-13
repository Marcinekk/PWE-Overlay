#pragma once
#include <cstdint>

namespace PWE::Hooks {
    struct RvaMappingWithdraw {
        uintptr_t rva;
        const char* label;
    };

    struct RvaMappingDeposit {
        uintptr_t rva;
        const char* label;
    };

    static constexpr RvaMappingDeposit kDepositRvaTable[] = {
        { 0x7477D1, "player_driver_income" },
        { 0xF4B5FC, "player_job_income" },
        { 0x546262, "player_truck_sold" },
        { 0x103A6A2, "player_loan_taken" },
        { 0xF2D950, "player_truck_sold_exchange" },
    };

    static constexpr RvaMappingWithdraw kWithdrawRvaTable[] = {
        { 0x7A388F,  "player_fined" },

        { 0xA1FF19,  "player_refuel" },
        { 0xA1E8AA,  "player_emergency_refuel" },

        { 0xFD1785,  "player_service_fix" },
        { 0x6BBF6D,  "player_service_mod" },
        { 0x1015BD2, "player_service_renovation" },
        { 0xA1E3E4,  "player_service_teleport" },
        { 0xFD3E12, "player_service_renovation" },

        { 0x6BC008,  "player_job_cancelled" },

        { 0xF2C539,  "player_truck_bought" },
        { 0xA3666C,  "player_truck_bought_used_online" },

        { 0x100F1C0, "player_garage_teleport" },
        { 0x7525A1,  "player_garage_upgrade" },
        { 0x7514C6,  "player_garage_bought" },
        { 0x7522AB, "player_garage_bought_online" },

        { 0x6BE923, "player_quick_travel" },

        { 0xF43D30,  "player_driver_hired" },

        { 0x103A7B7, "player_loan_paid" },
        { 0x74A5BC, "player_loan_paid_one" },

        { 0x7A4CF9, "player_tollgate_paid" },
    };

    inline const char* IdentifyWithdrawByRva(uintptr_t rva) {
        for (const auto& entry : kWithdrawRvaTable) {
            if (entry.rva == rva) return entry.label;
        }
        return nullptr;
    }

    inline const char* IdentifyDepositByRva(uintptr_t rva) {
        for (const auto& entry : kDepositRvaTable) {
            if (entry.rva == rva) return entry.label;
        }
        return nullptr;
    }
}