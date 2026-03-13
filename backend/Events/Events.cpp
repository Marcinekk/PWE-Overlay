#include "../PWEOverlay.hpp"
#include "../PWEWebView.hpp"
#include "Events.hpp"
#include <atomic>
#include <sstream>

namespace PWE::Events {
    namespace {
        std::atomic<int64_t> g_nextIdEvent{1};
        //
        std::string JsonEscape(const std::string& input) {
            std::ostringstream o;
            for (char c : input) {
                switch (c) {
                    case '"':  o << "\\\""; break;
                    case '\\': o << "\\\\"; break;
                    case '\b': o << "\\b";  break;
                    case '\f': o << "\\f";  break;
                    case '\n': o << "\\n";  break;
                    case '\r': o << "\\r";  break;
                    case '\t': o << "\\t";  break;
                    default:   o << c;
                }
            }
            return o.str();
        }
        //
        void CreateCustomEvent(bool income, int64_t amount, const char* description, const char* type) {
            const int64_t id = g_nextIdEvent++;
            //
            std::string safeDesc = description ? description : "";
            std::string safeType = type ? type : "";
            std::ostringstream ss;
            ss << "{\"type\":\"events/add\",\"payload\":{"
               << "\"id\":" << id
               << ",\"income\":\"" << (income ? "true" : "false") << "\""
               << ",\"amount\":" << amount
               << ",\"type\":\"" << JsonEscape(safeType) << "\""
               << ",\"description\":\"" << JsonEscape(safeDesc) << "\"}}";
            //
            std::string json = ss.str();
            PWE::PostWebViewOverlayMessageJson(json.c_str());
        }
    }  // namespace
    //
    void SendCustomEvent(bool income, int64_t amount, const char* description, const char* type) {
        CreateCustomEvent(income, amount, description, type);
    }
}  // namespace PWE::Events