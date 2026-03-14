#include "PWEOverlayInternal.hpp"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "../PWEOverlay.hpp"
#include "../PWEWebView.hpp"
#include "../Hooks/bank_deposit/DepositHook.hpp"
#include "../Hooks/bank_withdraw/WithdrawHook.hpp"

namespace PWE::Internal {
    namespace {
        bool ExtractStringValue(const std::string& json, const char* key, std::string& out) {
            if (!key || !key[0]) return false;

            const std::string marker = std::string("\"") + key + "\"";
            size_t keyPos = json.find(marker);
            if (keyPos == std::string::npos) return false;

            size_t colonPos = json.find(':', keyPos + marker.size());
            if (colonPos == std::string::npos) return false;
            size_t quoteStart = json.find('"', colonPos + 1);
            if (quoteStart == std::string::npos) return false;

            std::string value;
            bool escaped = false;
            for (size_t i = quoteStart + 1; i < json.size(); ++i) {
                const char c = json[i];
                if (escaped) {
                    value.push_back(c);
                    escaped = false;
                    continue;
                }
                if (c == '\\') {
                    escaped = true;
                    continue;
                }
                if (c == '"') {
                    out = value;
                    return true;
                }
                value.push_back(c);
            }

            return false;
        }

        bool ExtractInt64Value(const std::string& json, const char* key, int64_t& out) {
            if (!key || !key[0]) return false;

            const std::string marker = std::string("\"") + key + "\"";
            size_t keyPos = json.find(marker);
            if (keyPos == std::string::npos) return false;

            size_t colonPos = json.find(':', keyPos + marker.size());
            if (colonPos == std::string::npos) return false;

            size_t valuePos = colonPos + 1;
            while (valuePos < json.size() && std::isspace(static_cast<unsigned char>(json[valuePos]))) {
                ++valuePos;
            }
            if (valuePos >= json.size()) return false;

            const char* begin = json.c_str() + valuePos;
            char* end = nullptr;
            const long long parsed = std::strtoll(begin, &end, 10);
            if (end == begin) return false;
            out = static_cast<int64_t>(parsed);
            return true;
        }

        bool ExtractBoolValue(const std::string& json, const char* key, bool& out) {
            if (!key || !key[0]) return false;

            const std::string marker = std::string("\"") + key + "\"";
            size_t keyPos = json.find(marker);
            if (keyPos == std::string::npos) return false;

            size_t colonPos = json.find(':', keyPos + marker.size());
            if (colonPos == std::string::npos) return false;

            size_t valuePos = colonPos + 1;
            while (valuePos < json.size() && std::isspace(static_cast<unsigned char>(json[valuePos]))) {
                ++valuePos;
            }
            if (valuePos >= json.size()) return false;

            if (json.compare(valuePos, 4, "true") == 0) {
                out = true;
                return true;
            }
            if (json.compare(valuePos, 5, "false") == 0) {
                out = false;
                return true;
            }
            return false;
        }

        void SendCommandResponse(const std::string& requestId, bool ok, const char* message) {
            const int64_t balance = PWE::Hooks::GetMoney();
            char response[512] = {};
            std::snprintf(response, sizeof(response),
                          R"({"type":"plugin/response","payload":{"requestId":"%s","ok":%s,"message":"%s","balance":%lld}})",
                          requestId.c_str(), ok ? "true" : "false", message ? message : "",
                          static_cast<long long>(balance));
            PWE::PostWebViewOverlayMessageJson(response);
        }
    }  // namespace

    void HandleWebViewMessageJson(const char* jsonMessage) {
        if (!jsonMessage || !jsonMessage[0]) return;

        const std::string json(jsonMessage);
        std::string type;
        if (!ExtractStringValue(json, "type", type)) return;
        if (type != "plugin/command") return;

        std::string command;
        std::string requestId;
        int64_t amount = 0;
        ExtractStringValue(json, "command", command);
        ExtractStringValue(json, "requestId", requestId);
        ExtractInt64Value(json, "amount", amount);

        if (command == "economy/add_money") {
            if (amount <= 0) {
                SendCommandResponse(requestId, false, "amount_must_be_positive");
                return;
            }
            PWE::Hooks::AddMoney(amount);
            SendCommandResponse(requestId, true, "money_added");
            return;
        }

        if (command == "economy/remove_money") {
            if (amount <= 0) {
                SendCommandResponse(requestId, false, "amount_must_be_positive");
                return;
            }
            PWE::Hooks::RemoveMoney(amount);
            SendCommandResponse(requestId, true, "money_removed");
            return;
        }

        if (command == "overlay/toggle_focus") {
            ToggleWebViewFocus();
            SendCommandResponse(requestId, true, "focus_toggled");
            return;
        }

        if (command == "internal/get_versions") {
            char response[512] = {};
            std::snprintf(response, sizeof(response),
                R"({"type":"misc/versions","payload":{"requestId":"%s","ok":%s,"framework_mismatch":%s,"game_mismatch":%s}})",
                requestId.c_str(), "true", g_ctx.mismatchFrameworkVersion ? "true" : "false",
                g_ctx.mismatchGameVersion ? "true" : "false");
            PWE::PostWebViewOverlayMessageJson(response);
            return;
        }

        if (command == "internal/get_multiplayer") {
            char response[512] = {};
            std::snprintf(response, sizeof(response),
                R"({"type":"misc/multiplayer","payload":{"requestId":"%s","ok":%s,"is_multiplayer":%s}})",
                requestId.c_str(), "true", g_ctx.isMultiplayer ? "true" : "false");
            PWE::PostWebViewOverlayMessageJson(response);
            return;
        }

        SendCommandResponse(requestId, false, "unknown_command");
    }
}  // namespace PWE::Internal
