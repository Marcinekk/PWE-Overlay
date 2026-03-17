#pragma once
#include <cstdint>
#include <string>

namespace PWE::Storage {
    bool    Init(const char* dbPath);
    void    Close();
}