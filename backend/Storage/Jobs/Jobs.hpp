#pragma once
#include <cstdint>
#include <string>

namespace PWE::Storage::Jobs {
    void Clear();

    void Insert(
        const char* cargo,
        const char* srcCity,
        const char* srcCompany,
        const char* dstCity,
        const char* dstCompany,
        int64_t income,
        int32_t xp,
        float distance,
        const char* status,
        float cargoUnitMass,
        bool isSpecial
    );

    std::string QueryJson(int limit);
}