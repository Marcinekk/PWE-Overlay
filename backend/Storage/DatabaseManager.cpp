#include "DatabaseManager.hpp"
#include "../External/sqlite3/sqlite3.h"
#include "../PWEOverlay.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <sstream>

namespace PWE::Storage {
    namespace {
        sqlite3* g_db = nullptr;

        static const char* k_createTables = R"(
            CREATE TABLE IF NOT EXISTS transactions (
                id          INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp   INTEGER NOT NULL,
                amount      INTEGER NOT NULL,
                income      INTEGER NOT NULL,
                game        VARCHAR(5) NOT NULL DEFAULT '',
                profile     TEXT    NOT NULL DEFAULT '',
                type        TEXT    NOT NULL DEFAULT '',
                description TEXT    NOT NULL DEFAULT '',
                status      TEXT    NOT NULL DEFAULT 'pending'
            );

            CREATE TABLE IF NOT EXISTS settings (
                key     TEXT PRIMARY KEY,
                value   TEXT
            );

            INSERT OR IGNORE INTO settings (key, value)
            VALUES ('transaction_approve', 'false');
        )";

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

    bool Init(const char* dbPath) {
        if (g_db) return true;
        if (!dbPath || !dbPath[0]) return false;

        if (sqlite3_open(dbPath, &g_db) != SQLITE_OK) {
            g_db = nullptr;
            return false;
        }

        sqlite3_exec(g_db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

        char* errMsg = nullptr;
        if (sqlite3_exec(g_db, k_createTables, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
            sqlite3_close(g_db);
            g_db = nullptr;
            return false;
        }
        return true;
    }

    void Close() {
        if (g_db) {
            sqlite3_close(g_db);
            g_db = nullptr;
        }
    }

    void SetManualTransactionApprove(bool approve) {
        if (!g_db) return;

        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "UPDATE settings SET value='%s' WHERE key='transaction_approve';", approve ? "true" : "false");
        sqlite3_exec(g_db, sql, nullptr, nullptr, nullptr);
    }

    bool isManualTransactionApprove() {
        if (!g_db) return false;

        char* errMsg = nullptr;
        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "SELECT value FROM settings WHERE key='transaction_approve';");

        bool result = false;
        sqlite3_exec(g_db, sql, [](void* data, int argc, char** argv, char** azColName) -> int {
            if (argc >= 1 && argv[0]) *(bool*)data = (std::string(argv[0]) == "true");
            return 0;
        }, &result, &errMsg);

        if (errMsg) sqlite3_free(errMsg);
        return result;
    }

    void ClearTransactions() {
        if (!g_db) return;

        char gameCode[5] = {};
        if (g_ctx.environmentAPI) {
            g_ctx.environmentAPI->Env_GetGameCode(g_ctx.environmentHandle, gameCode, sizeof(gameCode));
        }

        char profileName[256] = {};
        if (g_ctx.environmentAPI) {
            g_ctx.environmentAPI->Env_GetActiveProfileName(g_ctx.environmentHandle, profileName, sizeof(profileName));
        }

        const std::string safeGame = EscapeSql(gameCode);
        const std::string safeProfile = EscapeSql(profileName);

        char sql[512] = {};
        std::snprintf(sql, sizeof(sql),
            "DELETE FROM transactions WHERE game='%s' AND profile='%s';",
            safeGame.c_str(), safeProfile.c_str());

        sqlite3_exec(g_db, sql, nullptr, nullptr, nullptr);
    }

    int64_t GetTransactionAmount(int64_t id) {
        if (!g_db) return 0;

        char sql[256] = {};
        std::snprintf(sql, sizeof(sql), "SELECT amount FROM transactions WHERE id=%lld;", static_cast<long long>(id));

        int64_t amount = 0;
        char* errMsg = nullptr;
        sqlite3_exec(g_db, sql, [](void* data, int argc, char** argv, char** azColName) -> int {
            if (argc >= 1 && argv[0]) *(int64_t*)data = std::stoll(argv[0]);
            return 0;
        }, &amount, &errMsg);

        if (errMsg) sqlite3_free(errMsg);
        return amount;
    }

    int64_t InsertTransaction(int64_t amount, bool income, const char* type, const char* description) {
        if (!g_db) return -1;

        char gameCode[5] = {};
        if (g_ctx.environmentAPI) {
            g_ctx.environmentAPI->Env_GetGameCode(g_ctx.environmentHandle, gameCode, sizeof(gameCode));
        }

        char profileName[256] = {};
        if (g_ctx.environmentAPI) {
            g_ctx.environmentAPI->Env_GetActiveProfileName(g_ctx.environmentHandle, profileName, sizeof(profileName));
        }

        const bool approve = isManualTransactionApprove();

        const int64_t ts = static_cast<int64_t>(std::time(nullptr)) * 1000LL;
        const std::string safeType = EscapeSql(type ? type : "");
        const std::string safeDesc = EscapeSql(description ? description : "");

        const std::string safeGame = EscapeSql(gameCode);
        const std::string safeProfile = EscapeSql(profileName);

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

        if (sqlite3_exec(g_db, sql, nullptr, nullptr, nullptr) != SQLITE_OK) return -1;
        return static_cast<int64_t>(sqlite3_last_insert_rowid(g_db));
    }

    bool SetStatus(int64_t id, const char* status) {
        if (!g_db || !status) return false;

        const std::string safeStatus = EscapeSql(status);
        char sql[256] = {};
        std::snprintf(sql, sizeof(sql),
            "UPDATE transactions SET status='%s' WHERE id=%lld;",
            safeStatus.c_str(),
            static_cast<long long>(id));

        return sqlite3_exec(g_db, sql, nullptr, nullptr, nullptr) == SQLITE_OK;
    }

    std::string QueryTransactionsJson(const char* statusFilter, int limit) {
        if (!g_db) return "[]";

        char gameCode[5] = {};
        if (g_ctx.environmentAPI) {
            g_ctx.environmentAPI->Env_GetGameCode(g_ctx.environmentHandle, gameCode, sizeof(gameCode));
        }

        char profileName[256] = {};
        if (g_ctx.environmentAPI) {
            g_ctx.environmentAPI->Env_GetActiveProfileName(g_ctx.environmentHandle, profileName, sizeof(profileName));
        }

        const std::string safeGame = EscapeSql(gameCode);
        const std::string safeProfile = EscapeSql(profileName);

        char sql[512] = {};
        if (statusFilter && statusFilter[0]) {
            const std::string safeFilter = EscapeSql(statusFilter);
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
        if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return "[]";

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
