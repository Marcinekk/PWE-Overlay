#pragma once
#include <cstdint>
#include <string>

namespace PWE::Storage {
    bool    Init(const char* dbPath);
    bool    SetStatus(int64_t id, const char* status);

    int64_t GetTransactionAmount(int64_t id);
    int64_t InsertTransaction(int64_t amount, bool income, const char* type, const char* description);

    void    SetManualTransactionApprove(bool approve);
    bool    isManualTransactionApprove();
    void    ClearTransactions();
    void    Close();

    std::string QueryTransactionsJson(const char* statusFilter, int limit);
}