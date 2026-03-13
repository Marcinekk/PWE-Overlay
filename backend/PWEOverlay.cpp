#include <cstring>
#include "Logitech/G27LEDManager.hpp"
#include "PWEOverlay.hpp"
#include "PWEWebView.hpp"
#include "Hooks/Hooks.hpp"
#include "Internal/PWEOverlayInternal.hpp"

namespace PWE {
    const char* PLUGIN_NAME = "PWEOverlay";
    PluginContext g_ctx;
    G27LEDManager g27LED;

    void BuildManifest(SPF_Manifest_Builder_Handle* h, const SPF_Manifest_Builder_API* api) {
        api->Info_SetName(h, PLUGIN_NAME);
        api->Info_SetVersion(h, "1.0.0");
        api->Info_SetAuthor(h, "PWE Overlay");
        api->Info_SetMinFrameworkVersion(h, "1.0.0");
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

        if (g_ctx.environmentHandle) {
            g_ctx.environmentAPI->Env_GetFrameworkVersion(g_ctx.environmentHandle, g_ctx.frameworkVersion, sizeof(g_ctx.frameworkVersion));
            g_ctx.environmentAPI->Env_GetGameVersion(g_ctx.environmentHandle, g_ctx.gameVersion, sizeof(g_ctx.gameVersion));
            g_ctx.environmentAPI->Env_GetGameName(g_ctx.environmentHandle, g_ctx.gameName, sizeof(g_ctx.gameName));

            if (strcmp(g_ctx.frameworkVersion, g_ctx.supportedFrameworkVersion) != 0) {
                g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: framework version mismatch.");
                g_ctx.mismatchFrameworkVersion = true;
            }
            if (strcmp(g_ctx.gameVersion, g_ctx.supportedATSVersion) != 0 && strcmp(g_ctx.gameName, "American Truck Simulator") == 0) {
                g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: ATS version mismatch.");
                g_ctx.mismatchGameVersion = true;
            }
            if (strcmp(g_ctx.gameVersion, g_ctx.supportedETS2Version) != 0 && strcmp(g_ctx.gameName, "Euro Truck Simulator 2") == 0) {
                g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: ETS2 version mismatch.");
                g_ctx.mismatchGameVersion = true;
            }
        }

        g27LED.Init();

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

    void OnUpdate() {
        Internal::ProcessHotkeys();
        Internal::UpdateG27LEDs();

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

            PWE::SetWebViewOverlayVisible(g_ctx.showWebView);
            Internal::ApplyFocusState();
            PWE::UpdateWebViewOverlayBounds();
            Internal::PushTelemetryToWebView();
        } else if (g_ctx.showWebView && g_ctx.loggerHandle && g_ctx.loadAPI) {
            g_ctx.loadAPI->logger->LogThrottled(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay.window_not_found", 2000, "PWEOverlay: game window not found.");
        }
    }

    void OnUnload() {
        PWE::SetWebViewOverlayMessageCallback(nullptr);
        PWE::ShutdownWebViewOverlay();
        Hooks::UnregisterAllHooks();
        g27LED.Shutdown();

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

    void OnSettingChanged(SPF_Config_Handle* config_handle, const char* keyPath) {
        if (!config_handle || !keyPath) return;

        g_ctx.configHandle = config_handle;
        if (g_ctx.showWebView) PWE::NavigateWebViewOverlay();
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
            exports->OnSettingChanged = OnSettingChanged;
            return true;
        }
    }
}  // namespace PWEOverlay