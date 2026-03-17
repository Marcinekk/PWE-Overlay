#include "Jobs.hpp"
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

namespace PWE::Storage::Jobs {
    void Clear() {
        if (!g_ctx.db) return;

        const std::string safeGame = Misc::EscapeSql(g_ctx.gameCode);
        const std::string safeProfile = Misc::EscapeSql(g_ctx.profileName);

        char sql[512] = {};
        std::snprintf(sql, sizeof(sql), "DELETE FROM logbook WHERE game='%s' AND profile='%s';", safeGame.c_str(), safeProfile.c_str());
        sqlite3_exec(g_ctx.db, sql, nullptr, nullptr, nullptr);
    }

    void Insert(const char* cargo, const char* srcCity, const char* srcCompany, const char* dstCity, const char* dstCompany, int64_t income, int32_t xp, float distance, const char* status, float cargoUnitMass, bool isSpecial) {
        if (!g_ctx.db) return;

        const int64_t ts = static_cast<int64_t>(std::time(nullptr)) * 1000LL;
        
        char sql[2048] = {};
        std::snprintf(sql, sizeof(sql),
            "INSERT INTO logbook (timestamp, game, profile, cargo, source_city, source_company, destination_city, destination_company, income, xp, distance, status, cargo_unit_mass, is_special) "
            "VALUES (%lld, '%s', '%s', '%s', '%s', '%s', '%s', '%s', %lld, %d, %f, '%s', %f, %d);",
            static_cast<long long>(ts),
            Misc::EscapeSql(g_ctx.gameCode).c_str(),
            Misc::EscapeSql(g_ctx.profileName).c_str(),
            Misc::EscapeSql(cargo ? cargo : "").c_str(),
            Misc::EscapeSql(srcCity ? srcCity : "").c_str(),
            Misc::EscapeSql(srcCompany ? srcCompany : "").c_str(),
            Misc::EscapeSql(dstCity ? dstCity : "").c_str(),
            Misc::EscapeSql(dstCompany ? dstCompany : "").c_str(),
            static_cast<long long>(income),
            xp,
            distance,
            Misc::EscapeSql(status ? status : "delivered").c_str(),
            cargoUnitMass,
            isSpecial ? 1 : 0
        );

        sqlite3_exec(g_ctx.db, sql, nullptr, nullptr, nullptr);
    }

    std::string QueryJson(int limit) {
        if (!g_ctx.db) return "[]";

        const std::string safeGame = Misc::EscapeSql(g_ctx.gameCode);
        const std::string safeProfile = Misc::EscapeSql(g_ctx.profileName);

        char sql[1024] = {};
        if (limit > 0) {
            std::snprintf(sql, sizeof(sql),
                "SELECT id,timestamp,cargo,source_city,source_company,destination_city,destination_company,income,xp,distance,status,cargo_unit_mass,is_special "
                "FROM logbook WHERE game='%s' AND profile='%s' ORDER BY id DESC LIMIT %d;",
                safeGame.c_str(), safeProfile.c_str(), limit);
        } else {
            std::snprintf(sql, sizeof(sql),
                "SELECT id,timestamp,cargo,source_city,source_company,destination_city,destination_company,income,xp,distance,status,cargo_unit_mass,is_special "
                "FROM logbook WHERE game='%s' AND profile='%s' ORDER BY id DESC;",
                safeGame.c_str(), safeProfile.c_str());
        }

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(g_ctx.db, sql, -1, &stmt, nullptr) != SQLITE_OK) return "[]";

        std::ostringstream out;
        out << "[";
        bool first = true;

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

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (!first) out << ",";
            first = false;

            out << "{"
                << "\"id\":" << sqlite3_column_int64(stmt, 0) << ","
                << "\"timestamp\":" << sqlite3_column_int64(stmt, 1) << ","
                << "\"cargo\":\"" << escJson((const char*)sqlite3_column_text(stmt, 2)) << "\","
                << "\"sourceCity\":\"" << escJson((const char*)sqlite3_column_text(stmt, 3)) << "\","
                << "\"sourceCompany\":\"" << escJson((const char*)sqlite3_column_text(stmt, 4)) << "\","
                << "\"destinationCity\":\"" << escJson((const char*)sqlite3_column_text(stmt, 5)) << "\","
                << "\"destinationCompany\":\"" << escJson((const char*)sqlite3_column_text(stmt, 6)) << "\","
                << "\"cargo_unit_mass\":" << sqlite3_column_double(stmt, 10) << ","
                << "\"is_special\":" << sqlite3_column_int(stmt, 11) << ","
                << "\"income\":" << sqlite3_column_int64(stmt, 7) << ","
                << "\"xp\":" << sqlite3_column_int(stmt, 8) << ","
                << "\"distance\":" << sqlite3_column_double(stmt, 9) << ","
                << "\"status\":\"" << escJson((const char*)sqlite3_column_text(stmt, 10)) << "\""
                << "}";
        }

        sqlite3_finalize(stmt);
        out << "]";
        return out.str();
    }
}