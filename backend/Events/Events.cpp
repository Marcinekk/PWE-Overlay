#include "../PWEOverlay.hpp"
#include "../PWEWebView.hpp"
#include "Events.hpp"
#include "../Internal/PWEOverlayInternal.hpp"
#include <atomic>
#include <sstream>

namespace PWE::Events {
    namespace {
        std::atomic<int64_t> g_nextIdEvent{1};
        void CreateCustomEvent(bool income, int64_t amount, const char* description, const char* type) {
            const int64_t id = g_nextIdEvent++;

            std::string safeDesc = description ? description : "";
            std::string safeType = type ? type : "";
            std::ostringstream ss;
            ss << "{\"type\":\"events/add\",\"payload\":{"
               << "\"id\":" << id
               << ",\"income\":\"" << (income ? "true" : "false") << "\""
               << ",\"amount\":" << amount
               << ",\"type\":\"" << PWE::Internal::JsonEscape(safeType.c_str()) << "\""
               << ",\"description\":\"" << PWE::Internal::JsonEscape(safeDesc.c_str()) << "\"}}";

            std::string json = ss.str();
            PWE::PostWebViewOverlayMessageJson(json.c_str());
        }
    }  // namespace

    void SendCustomEvent(bool income, int64_t amount, const char* description, const char* type) {
        CreateCustomEvent(income, amount, description, type);
    }
}  // namespace PWE::Events