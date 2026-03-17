#pragma once
#include <cstdint>
#include <string>

namespace PWE::Storage::Transactions {
    bool    SetStatus(int64_t id, const char* status);
    bool    isManual();

    int64_t GetAmount(int64_t id);
    int64_t Insert(int64_t amount, bool income, const char* type, const char* description);

    bool    SetManual(bool approve);
    void    Clear();

    std::string QueryJson(const char* statusFilter, int limit);
}