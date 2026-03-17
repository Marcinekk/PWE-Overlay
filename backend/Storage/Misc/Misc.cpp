#include "Misc.hpp"
#include "../DatabaseManager.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace PWE::Storage::Misc {
    std::string EscapeSql(const char* src) {
        if (!src) return "";
        std::string out;
        for (const char* p = src; *p; ++p) {
            if (*p == '\'') out += "''";
            else out += *p;
        }
        return out;
    }
}