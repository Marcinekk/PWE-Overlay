#include "Transactions.hpp"
#include "../DatabaseManager.hpp"
#include "../Misc/Misc.hpp"
#include "../../External/sqlite3/sqlite3.h"
#include "../../PWEOverlay.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <sstream>

namespace PWE::Storage::Transactions {
    bool isManual() {
        if (!g_ctx.db) return false;

        char* errMsg = nullptr;
        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "SELECT value FROM settings WHERE key='transaction_approve';");

        bool result = false;
        sqlite3_exec(g_ctx.db, sql, [](void* data, int argc, char** argv, char** azColName) -> int {
            if (argc >= 1 && argv[0]) *(bool*)data = (std::string(argv[0]) == "true");
            return 0;
        }, &result, &errMsg);

        if (errMsg) sqlite3_free(errMsg);
        return result;
    }

    bool SetStatus(int64_t id, const char* status) {
        if (!g_ctx.db || !status) return false;

        const std::string safeStatus = Misc::EscapeSql(status);
        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "UPDATE transactions SET status='%s' WHERE id=%lld;", safeStatus.c_str(), static_cast<long long>(id));
        return sqlite3_exec(g_ctx.db, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
    }

    bool SetManual(bool approve) {
        if (!g_ctx.db) return false;

        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "UPDATE settings SET value='%s' WHERE key='transaction_approve';", approve ? "true" : "false");
        return sqlite3_exec(g_ctx.db, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
    }

    void Clear() {
        if (!g_ctx.db) return;

        const std::string safeGame = Misc::EscapeSql(g_ctx.gameCode);
        const std::string safeProfile = Misc::EscapeSql(g_ctx.profileName);

        char sql[512] = {};
        std::snprintf(sql, sizeof(sql), "DELETE FROM transactions WHERE game='%s' AND profile='%s';", safeGame.c_str(), safeProfile.c_str());
        sqlite3_exec(g_ctx.db, sql, nullptr, nullptr, nullptr);
    }

    int64_t GetAmount(int64_t id) {
        if (!g_ctx.db) return 0;

        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "SELECT amount FROM transactions WHERE id=%lld;", static_cast<long long>(id));

        int64_t amount = 0;
        char* errMsg = nullptr;
        sqlite3_exec(g_ctx.db, sql, [](void* data, int argc, char** argv, char** azColName) -> int {
            if (argc >= 1 && argv[0]) *(int64_t*)data = std::stoll(argv[0]);
            return 0;
        }, &amount, &errMsg);

        if (errMsg) sqlite3_free(errMsg);
        return amount;
    }

    int64_t Insert(int64_t amount, bool income, const char* type, const char* description) {
        if (!g_ctx.db) return -1;

        const bool approve = isManual();

        const int64_t ts = static_cast<int64_t>(std::time(nullptr)) * 1000LL;
        const std::string safeType = Misc::EscapeSql(type ? type : "");
        const std::string safeDesc = Misc::EscapeSql(description ? description : "");

        const std::string safeGame = Misc::EscapeSql(g_ctx.gameCode);
        const std::string safeProfile = Misc::EscapeSql(g_ctx.profileName);

        char sql[1024] = {};
        std::snprintf(sql, sizeof(sql),
            "INSERT INTO transactions (timestamp, amount, income, game, profile, type, description, status) "
            "VALUES (%lld, %lld, %d, '%s', '%s', '%s', '%s', '%s');",
            static_cast<long long>(ts),
            static_cast<long long>(amount),
            income ? 1 : 0,
            safeGame.c_str(),
            safeProfile.c_str(),
            safeType.c_str(),
            safeDesc.c_str(),
            approve ? "pending" : "approved"
        );

        if (sqlite3_exec(g_ctx.db, sql, nullptr, nullptr, nullptr) != SQLITE_OK) return -1;
        return static_cast<int64_t>(sqlite3_last_insert_rowid(g_ctx.db));
    }

    std::string QueryJson(const char* statusFilter, int limit) {
        if (!g_ctx.db) return "[]";

        const std::string safeGame = Misc::EscapeSql(g_ctx.gameCode);
        const std::string safeProfile = Misc::EscapeSql(g_ctx.profileName);

        char sql[512] = {};
        if (statusFilter && statusFilter[0]) {
            const std::string safeFilter = Misc::EscapeSql(statusFilter);
            if (limit > 0) {
                std::snprintf(sql, sizeof(sql),
                    "SELECT id,timestamp,amount,income,game,profile,type,description,status "
                    "FROM transactions WHERE status='%s' AND game='%s' AND profile='%s' ORDER BY id DESC LIMIT %d;",
                    safeFilter.c_str(), safeGame.c_str(), safeProfile.c_str(), limit);
            } else {
                std::snprintf(sql, sizeof(sql),
                    "SELECT id,timestamp,amount,income,game,profile,type,description,status "
                    "FROM transactions WHERE status='%s' AND game='%s' AND profile='%s' ORDER BY id DESC;",
                    safeFilter.c_str(), safeGame.c_str(), safeProfile.c_str());
            }
        } else {
            if (limit > 0) {
                std::snprintf(sql, sizeof(sql),
                    "SELECT id,timestamp,amount,income,game,profile,type,description,status "
                    "FROM transactions WHERE game='%s' AND profile='%s' ORDER BY id DESC LIMIT %d;",
                    safeGame.c_str(), safeProfile.c_str(), limit);
            } else {
                std::snprintf(sql, sizeof(sql),
                    "SELECT id,timestamp,amount,income,game,profile,type,description,status "
                    "FROM transactions WHERE game='%s' AND profile='%s' ORDER BY id DESC;",
                    safeGame.c_str(), safeProfile.c_str());
            }
        }

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(g_ctx.db, sql, -1, &stmt, nullptr) != SQLITE_OK) return "[]";

        std::ostringstream out;
        out << "[";
        bool first = true;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!first) out << ",";
            first = false;

            const int64_t id       = sqlite3_column_int64(stmt, 0);
            const int64_t ts       = sqlite3_column_int64(stmt, 1);
            const int64_t amount   = sqlite3_column_int64(stmt, 2);
            const int     income   = sqlite3_column_int(stmt, 3);
            const char*   game     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            const char*   profile  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            const char*   type     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            const char*   desc     = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            const char*   status   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

            auto escJson = [](const char* s) -> std::string {
                if (!s) return "";
                std::string r;
                for (const unsigned char c : std::string(s)) {
                    switch (c) {
                        case '"':  r += "\\\""; break;
                        case '\\': r += "\\\\"; break;
                        case '\n': r += "\\n";  break;
                        case '\r': r += "\\r";  break;
                        case '\t': r += "\\t";  break;
                        default:   if (c >= 0x20) r += static_cast<char>(c); break;
                    }
                }
                return r;
            };

            out << "{"
                << "\"id\":"          << id     << ","
                << "\"timestamp\":"   << ts     << ","
                << "\"amount\":"      << amount << ","
                << "\"income\":"      << (income ? "true" : "false") << ","
                << "\"game\":\""      << escJson(game)   << "\","
                << "\"profile\":\""   << escJson(profile) << "\","
                << "\"type\":\""      << escJson(type)   << "\","
                << "\"description\":\"" << escJson(desc) << "\","
                << "\"status\":\""    << escJson(status) << "\""
                << "}";
        }

        sqlite3_finalize(stmt);
        out << "]";
        return out.str();
    }
}