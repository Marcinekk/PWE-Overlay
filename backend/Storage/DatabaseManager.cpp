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

            CREATE TABLE IF NOT EXISTS logbook (
                id                  INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp           INTEGER NOT NULL,
                game                VARCHAR(5) NOT NULL DEFAULT '',
                profile             TEXT    NOT NULL DEFAULT '',
                cargo               TEXT    NOT NULL DEFAULT '',
                is_special          INTEGER NOT NULL DEFAULT 0,
                source_city         TEXT    NOT NULL DEFAULT '',
                source_company      TEXT    NOT NULL DEFAULT '',
                destination_city    TEXT    NOT NULL DEFAULT '',
                destination_company TEXT    NOT NULL DEFAULT '',
                income              INTEGER NOT NULL DEFAULT 0,
                xp                  INTEGER NOT NULL DEFAULT 0,
                cargo_unit_mass     REAL    NOT NULL DEFAULT 0,
                distance            REAL    NOT NULL DEFAULT 0,
                status              TEXT    NOT NULL DEFAULT 'delivered'
            );

            INSERT OR IGNORE INTO settings (key, value)
            VALUES ('transaction_approve', 'false');
        )";
    }

    bool Init(const char* dbPath) {
        if (g_ctx.db) return true;
        if (!dbPath || !dbPath[0]) return false;

        if (sqlite3_open(dbPath, &g_ctx.db) != SQLITE_OK) {
            g_ctx.db = nullptr;
            return false;
        }

        sqlite3_exec(g_ctx.db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);

        char* errMsg = nullptr;
        if (sqlite3_exec(g_ctx.db, k_createTables, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            sqlite3_free(errMsg);
            sqlite3_close(g_ctx.db);
            g_ctx.db = nullptr;
            return false;
        }
        return true;
    }

    void Close() {
        if (g_ctx.db) {
            sqlite3_close(g_ctx.db);
            g_ctx.db = nullptr;
        }
    }
}
