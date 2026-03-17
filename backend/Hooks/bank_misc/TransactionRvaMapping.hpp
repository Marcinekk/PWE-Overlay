#pragma once
#include <cstdint>

namespace PWE::Hooks::Economy {
    struct RvaMappingWithdraw {
        uintptr_t rva;
        const char* label;
    };

    struct RvaMappingDeposit {
        uintptr_t rva;
        const char* label;
    };

    // ATS TABLE
    static constexpr RvaMappingDeposit kDepositRvaTableATS[] = {
        { 0x7477D1, "player_driver_income" },
        { 0xF4B5FC, "player_job_income" },
        { 0x546262, "player_truck_sold" },
        { 0x103A6A2, "player_loan_taken" },
        { 0xF2D950, "player_truck_sold_exchange" },
        { 0x75175A, "player_garage_sold" },
        { 0x53E3E7, "player_trailer_sold" },
    };

    static constexpr RvaMappingWithdraw kWithdrawRvaTableATS[] = {
        { 0x7A388F, "player_fined" },

        { 0xA1FF19, "player_refuel" },
        { 0xA1E8AA, "player_emergency_refuel" },

        { 0xFD1785, "player_service_fix" },
        { 0x6BBF6D, "player_service_mod" },
        { 0x1015BD2, "player_service_renovation" },
        { 0xA1E3E4, "player_service_teleport" },
        { 0xFD3E12, "player_service_renovation" },

        { 0xA36B3D, "player_trailer_bought" },

        { 0x6BC008, "player_job_cancelled" },

        { 0xF2C539, "player_truck_bought" },
        { 0xA3666C, "player_truck_bought_used_online" },

        { 0x100F1C0, "player_garage_teleport" },
        { 0x7525A1, "player_garage_upgrade" },
        { 0x7514C6, "player_garage_bought" },
        { 0x7522AB, "player_garage_bought_online" },

        { 0x6BE923, "player_quick_travel" },

        { 0xF43D30, "player_driver_hired" },
        { 0x7477F3, "player_driver_expense" },

        { 0x103A7B7, "player_loan_paid" },
        { 0x74A5BC, "player_loan_paid_one" },

        { 0x7A4CF9, "player_tollgate_paid" },
    };

    // ETS2 TABLE
    static constexpr RvaMappingDeposit kDepositRvaTableETS[] = {
        { 0x7477C1, "player_driver_income" },
        { 0xF4B6FC, "player_job_income" },
        { 0x546252, "player_truck_sold" },
        { 0x103A7B2, "player_loan_taken" },
        { 0xF2DA50, "player_truck_sold_exchange" },
        { 0x75174A, "player_garage_sold" },
        { 0x53E3D7, "player_trailer_sold" },
    };

    static constexpr RvaMappingWithdraw kWithdrawRvaTableETS[] = {
        { 0x7A387F, "player_fined" },

        { 0xA1FF09, "player_refuel" },
        { 0xA1E89A, "player_emergency_refuel" },

        { 0xFD1895, "player_service_fix" },
        { 0x6BBF5D, "player_service_mod" },
        { 0xFD3F22, "player_service_renovation" },
        { 0xA1E3D4, "player_service_teleport" },
        { 0x1015CE2, "player_service_renovation" },

        { 0xA36B2D, "player_trailer_bought" },

        { 0x7477C1, "player_job_cancelled" },

        { 0xF2C639, "player_truck_bought" },
        { 0xA3665C, "player_truck_bought_used_online" },

        { 0x100F2D0, "player_garage_teleport" },
        { 0x752591, "player_garage_upgrade" },
        { 0x7514B6, "player_garage_bought" },
        { 0x75229B, "player_garage_bought_online" },

        { 0x6BE913, "player_quick_travel" },

        { 0xF43E30, "player_driver_hired" },
        { 0x7477E3, "player_driver_expense" },

        { 0x103A8C7, "player_loan_paid" },
        { 0x74A5AC, "player_loan_paid_one" },

        { 0x7A4CE9, "player_tollgate_paid" },
        { 0xA359A2, "player_ferry_paid" },
        { 0xA359A2, "player_train_paid" },
    };

    template<typename T, size_t N>
    inline const char* FindByRva(uintptr_t rva, const T (&table)[N]) {
        for (const auto& entry : table) {
            if (entry.rva == rva) return entry.label;
        }
        return nullptr;
    }

    inline const char* IdentifyWithdrawByRva(uintptr_t rva, bool isATS) {
        return isATS ? FindByRva(rva, kWithdrawRvaTableATS) : FindByRva(rva, kWithdrawRvaTableETS);
    }

    inline const char* IdentifyDepositByRva(uintptr_t rva, bool isATS) {
        return isATS ? FindByRva(rva, kDepositRvaTableATS) : FindByRva(rva, kDepositRvaTableETS);
    }
}