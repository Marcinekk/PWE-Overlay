#include <cstring>
#include <cstdio>
#include <cctype>
#include <string>
#include <shlobj.h>
#include "Logitech/G27LEDManager.hpp"
#include "PWEOverlay.hpp"
#include "PWEWebView.hpp"
#include "Hooks/Hooks.hpp"
#include "Internal/PWEOverlayInternal.hpp"
#include "Events/Events.hpp"
#include "Storage/DatabaseManager.hpp"

namespace PWE {
    const char* PLUGIN_NAME = "PWEOverlay";
    PluginContext g_ctx;
    G27LEDManager g27LED;

    namespace {
        bool g_versionMismatchLogged = false;

        std::string ExtractNumericVersionPrefix(const char* version) {
            if (!version) return {};
            while (*version && std::isspace(static_cast<unsigned char>(*version))) ++version;

            std::string out;
            out.reserve(32);
            for (const char* p = version; *p; ++p) {
                const unsigned char ch = static_cast<unsigned char>(*p);
                if (std::isdigit(ch) || ch == '.') out.push_back(static_cast<char>(ch));
                else break;
            }
            while (!out.empty() && out.back() == '.') out.pop_back();
            return out;
        }

        bool VersionCompatible(const char* actual, const char* expected) {
            const std::string a = ExtractNumericVersionPrefix(actual);
            const std::string e = ExtractNumericVersionPrefix(expected);
            if (a.empty() || e.empty()) return false;
            if (a == e) return true;
            if (a.rfind(e + ".", 0) == 0) return true;
            return false;
        }

        void RefreshVersionMismatchState() {
            if (!g_ctx.environmentAPI || !g_ctx.environmentHandle) return;

            char frameworkVersion[64] = {};
            char gameVersion[64] = {};
            char gameName[128] = {};

            char gameCode[16] = {};

            g_ctx.environmentAPI->Env_GetFrameworkVersion(g_ctx.environmentHandle, frameworkVersion, sizeof(frameworkVersion));
            g_ctx.environmentAPI->Env_GetGameVersion(g_ctx.environmentHandle, gameVersion, sizeof(gameVersion));
            g_ctx.environmentAPI->Env_GetGameName(g_ctx.environmentHandle, gameName, sizeof(gameName));
            g_ctx.environmentAPI->Env_GetGameCode(g_ctx.environmentHandle, gameCode, sizeof(gameCode));

            if (gameCode[0]) {
                std::strncpy(g_ctx.gameCode, gameCode, sizeof(g_ctx.gameCode) - 1);
                g_ctx.gameCode[sizeof(g_ctx.gameCode) - 1] = '\0';
            }

            if (frameworkVersion[0]) {
                std::strncpy(g_ctx.frameworkVersion, frameworkVersion, sizeof(g_ctx.frameworkVersion) - 1);
                g_ctx.frameworkVersion[sizeof(g_ctx.frameworkVersion) - 1] = '\0';
            }

            if (gameVersion[0]) {
                std::strncpy(g_ctx.gameVersion, gameVersion, sizeof(g_ctx.gameVersion) - 1);
                g_ctx.gameVersion[sizeof(g_ctx.gameVersion) - 1] = '\0';
            }

            if (gameName[0]) {
                std::strncpy(g_ctx.gameName, gameName, sizeof(g_ctx.gameName) - 1);
                g_ctx.gameName[sizeof(g_ctx.gameName) - 1] = '\0';
            }

            if (frameworkVersion[0] && g_ctx.supportedFrameworkVersion) {
                g_ctx.mismatchFrameworkVersion = !VersionCompatible(frameworkVersion, g_ctx.supportedFrameworkVersion);
            }

            if (gameCode[0] && gameVersion[0]) {
                if (std::strcmp(gameCode, "ats") == 0) {
                    g_ctx.mismatchGameVersion = !VersionCompatible(gameVersion, g_ctx.supportedATSVersion);
                } else if (std::strcmp(gameCode, "eut2") == 0) {
                    g_ctx.mismatchGameVersion = !VersionCompatible(gameVersion, g_ctx.supportedETS2Version);
                } else {
                    g_ctx.mismatchGameVersion = false;
                }
            }
        }

        void LogMismatchOnceIfNeeded() {
            if (g_versionMismatchLogged) return;
            if (!g_ctx.loggerHandle || !g_ctx.loadAPI) return;
            if (!g_ctx.mismatchFrameworkVersion && !g_ctx.mismatchGameVersion) return;

            char msg[512] = {};
            if (g_ctx.formattingAPI) {
                g_ctx.formattingAPI->Fmt_Format(msg, sizeof(msg),
                              "PWEOverlay: version mismatch (framework=%s, game=%s, fw=%s, gameVer=%s).",
                              g_ctx.mismatchFrameworkVersion ? "true" : "false",
                              g_ctx.mismatchGameVersion ? "true" : "false",
                              g_ctx.frameworkVersion[0] ? g_ctx.frameworkVersion : "unknown",
                              g_ctx.gameVersion[0] ? g_ctx.gameVersion : "unknown");
                g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, msg);
            }
            g_versionMismatchLogged = true;
        }
    }  // namespace

    void BuildManifest(SPF_Manifest_Builder_Handle* h, const SPF_Manifest_Builder_API* api) {
        api->Info_SetName(h, PLUGIN_NAME);
        api->Info_SetVersion(h, "1.3");
        api->Info_SetAuthor(h, "PWE");
        api->Info_SetGithubUrl(h, "https://github.com/Marcinekk/PWE-Overlay");
        api->Info_SetWebsiteUrl(h, "https://ko-fi.com/pwe_scripts");
        api->Info_SetMinFrameworkVersion(h, g_ctx.supportedFrameworkVersion);
        api->Info_SetDescriptionLiteral(h, "WebView overlay plugin for SPF.");

        api->Policy_SetAllowUserConfig(h, true);
        api->Policy_AddConfigurableSystem(h, "settings");
        api->Policy_AddConfigurableSystem(h, "keybinds");

        api->Defaults_AddKeybind(h, PLUGIN_NAME, "toggle_webview", "keyboard", "KEY_INSERT", "always");
        api->Defaults_AddKeybind(h, PLUGIN_NAME, "toggle_focus", "keyboard", "KEY_HOME", "always");
        api->Defaults_AddKeybind(h, PLUGIN_NAME, "toggle_action", "keyboard", "KEY_END", "always");

        api->Meta_AddKeybind(h, PLUGIN_NAME, "toggle_webview", "Toggle WebView", "Show or hide WebView overlay.");
        api->Meta_AddKeybind(h, PLUGIN_NAME, "toggle_focus", "Toggle focus", "Switch focus between game and WebView.");
        api->Meta_AddKeybind(h, PLUGIN_NAME, "toggle_action", "Toggle action", "Toggle overlay action.");
    }

    void OnLoad(const SPF_Load_API* load_api) {
        g_ctx.loadAPI = load_api;
        if (!g_ctx.loadAPI) return;

        g_ctx.loggerHandle = g_ctx.loadAPI->logger->Log_GetContext(PLUGIN_NAME);
        g_ctx.formattingAPI = g_ctx.loadAPI->formatting;

        if (g_ctx.loadAPI->config) g_ctx.configHandle = g_ctx.loadAPI->config->Cfg_GetContext(PLUGIN_NAME);

        g_ctx.environmentAPI = g_ctx.loadAPI->environment;
        if (g_ctx.environmentAPI) g_ctx.environmentHandle = g_ctx.environmentAPI->Env_GetContext(PLUGIN_NAME);

        RefreshVersionMismatchState();

        g27LED.Init();

        {
            char appData[MAX_PATH] = {};
            if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, appData))) {
                std::string dbDir = std::string(appData) + "\\PWE-Overlay";
                CreateDirectoryA(dbDir.c_str(), nullptr);
                std::string dbPath = dbDir + "\\database.db";
                PWE::Storage::Init(dbPath.c_str());
            }
        }

        if (g_ctx.loggerHandle) {
            if (!PWE::IsWebViewOverlayAvailable()) {
                g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: WebView2 headers not found at build time; overlay disabled.");
            }
        }

        PWE::SetWebViewOverlayMessageCallback(&Internal::HandleWebViewMessageJson);
    }

    void OnActivated(const SPF_Core_API* core_api) {
        g_ctx.coreAPI = core_api;
        if (!g_ctx.coreAPI) return;

        Hooks::RegisterAllHooks();

        if (!g_ctx.environmentAPI) g_ctx.environmentAPI = g_ctx.coreAPI->environment;
        if (!g_ctx.environmentHandle && g_ctx.environmentAPI) g_ctx.environmentHandle = g_ctx.environmentAPI->Env_GetContext(PLUGIN_NAME);
        RefreshVersionMismatchState();
        LogMismatchOnceIfNeeded();
        if (g_ctx.coreAPI->telemetry) g_ctx.telemetryHandle = g_ctx.coreAPI->telemetry->Tel_GetContext(PLUGIN_NAME);
        if (g_ctx.coreAPI->keybinds) {
            g_ctx.keybindsHandle = g_ctx.coreAPI->keybinds->Kbind_GetContext(PLUGIN_NAME);
            if (g_ctx.keybindsHandle) {
                g_ctx.coreAPI->keybinds->Kbind_Register(g_ctx.keybindsHandle, "PWEOverlay.toggle_webview", &Internal::ToggleWebViewVisibility);
                g_ctx.coreAPI->keybinds->Kbind_Register(g_ctx.keybindsHandle, "PWEOverlay.toggle_focus", &Internal::ToggleWebViewFocus);
                g_ctx.coreAPI->keybinds->Kbind_Register(g_ctx.keybindsHandle, "PWEOverlay.toggle_action", &Internal::ToggleAction);
            }
        }
    }

    void OnRegisterUI(SPF_UI_API* ui_api) {
        g_ctx.uiAPI = ui_api;
    }

    void OnGameWorldReady() {
        Events::RegisterEvents();
    }

    void OnUpdate() {
        static bool lastRequestedVisibleSet = false;
        static bool lastRequestedVisible = false;
        static bool versionsChecked = false;

        Internal::ProcessHotkeys();
        Internal::UpdateG27LEDs();

        if (g_ctx.environmentAPI && g_ctx.environmentHandle) {
            if (!versionsChecked) {
                RefreshVersionMismatchState();
                LogMismatchOnceIfNeeded();
                versionsChecked = true;
            }

            char buffer[512] = {};

            g_ctx.environmentAPI->Env_GetActiveProfileName(g_ctx.environmentHandle, buffer, sizeof(buffer));
            if (buffer[0] && std::strcmp(buffer, g_ctx.profileName) != 0) {
                std::strncpy(g_ctx.profileName, buffer, sizeof(g_ctx.profileName) - 1);
                g_ctx.profileName[sizeof(g_ctx.profileName) - 1] = '\0';
            }

            g_ctx.environmentAPI->Env_GetGameName(g_ctx.environmentHandle, buffer, sizeof(buffer));
            if (buffer[0] && std::strcmp(buffer, g_ctx.gameName) != 0) {
                std::strncpy(g_ctx.gameName, buffer, sizeof(g_ctx.gameName) - 1);
                g_ctx.gameName[sizeof(g_ctx.gameName) - 1] = '\0';
            }

            if (g_ctx.environmentAPI->Env_GetMultiplayerStatus(g_ctx.environmentHandle, buffer, sizeof(buffer))) {
                const bool isMp = (buffer[0] != '\0' && std::strcmp(buffer, "None") != 0);
                if (isMp != g_ctx.isMultiplayer) g_ctx.isMultiplayer = isMp;
            }
        }

        if (g_ctx.showWebView) Internal::sendMiscData();

        if (!PWE::IsWebViewOverlayAvailable()) {
            if (!g_ctx.webViewUnavailableLogged && g_ctx.showWebView && g_ctx.loggerHandle && g_ctx.loadAPI) {
                g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN,
                                           "PWEOverlay: open requested but WebView2 is unavailable in this build.");
                g_ctx.webViewUnavailableLogged = true;
            }
            return;
        }

        HWND hostWindow = Internal::ResolveGameWindow();
        if (hostWindow) {
            const bool ready = PWE::InitializeWebViewOverlay(hostWindow);
            if (ready && !g_ctx.webViewInitLogged && g_ctx.loggerHandle && g_ctx.loadAPI) g_ctx.webViewInitLogged = true;

            if (!lastRequestedVisibleSet || lastRequestedVisible != g_ctx.showWebView) {
                PWE::SetWebViewOverlayVisible(g_ctx.showWebView);
                lastRequestedVisible = g_ctx.showWebView;
                lastRequestedVisibleSet = true;
            }
            Internal::ApplyFocusState();
            if (g_ctx.showWebView) PWE::UpdateWebViewOverlayBounds();
            if (g_ctx.showWebView && !g_ctx.webViewFocus) PWE::TickWebViewOverlayFocusGuard();
            Internal::PushTelemetryToWebView();
        } else if (g_ctx.showWebView && g_ctx.loggerHandle && g_ctx.loadAPI) {
            g_ctx.loadAPI->logger->LogThrottled(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay.window_not_found", 2000, "PWEOverlay: game window not found.");
        }
    }

    void OnUnload() {
        PWE::SetWebViewOverlayMessageCallback(nullptr);
        PWE::ShutdownWebViewOverlay();

        Hooks::UnregisterAllHooks();
        Events::UnregisterEvents();

        g27LED.Shutdown();
        PWE::Storage::Close();

        g_ctx.coreAPI = nullptr;
        g_ctx.loadAPI = nullptr;
        g_ctx.loggerHandle = nullptr;
        g_ctx.keybindsHandle = nullptr;
        g_ctx.configHandle = nullptr;
        g_ctx.environmentHandle = nullptr;
        g_ctx.telemetryHandle = nullptr;
        g_ctx.formattingAPI = nullptr;
        g_ctx.uiAPI = nullptr;
        g_ctx.gameWindow = nullptr;
    }

    extern "C" {
        SPF_PLUGIN_EXPORT bool SPF_GetManifestAPI(SPF_Manifest_API* out_api) {
            if (!out_api) return false;
            out_api->BuildManifest = BuildManifest;
            return true;
        }

        SPF_PLUGIN_EXPORT bool SPF_GetPlugin(SPF_Plugin_Exports* exports) {
            if (!exports) return false;
            exports->OnLoad = OnLoad;
            exports->OnActivated = OnActivated;
            exports->OnUnload = OnUnload;
            exports->OnUpdate = OnUpdate;
            exports->OnRegisterUI = OnRegisterUI;
            exports->OnGameWorldReady = OnGameWorldReady;
            return true;
        }
    }
}  // namespace PWEOverlay
