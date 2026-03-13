#pragma once
//
#include <windows.h>
#include <SPF_API/SPF_Plugin.h>
#include <SPF_API/SPF_Manifest_API.h>
#include <SPF_API/SPF_Logger_API.h>
#include <SPF_API/SPF_Formatting_API.h>
#include <SPF_API/SPF_UI_API.h>
#include <SPF_API/SPF_KeyBinds_API.h>
#include <SPF_API/SPF_Config_API.h>
#include <SPF_API/SPF_Environment_API.h>
#include <SPF_API/SPF_Telemetry_API.h>
#include <SPF_API/SPF_GameConsole_API.h>
//
namespace PWE {
    struct PluginContext {
        const SPF_Load_API* loadAPI = nullptr;
        const SPF_Core_API* coreAPI = nullptr;

        SPF_Logger_Handle* loggerHandle = nullptr;
        SPF_KeyBinds_Handle* keybindsHandle = nullptr;
        SPF_Config_Handle* configHandle = nullptr;
        SPF_Environment_Handle* environmentHandle = nullptr;
        SPF_Telemetry_Handle* telemetryHandle = nullptr;

        const SPF_Formatting_API* formattingAPI = nullptr;
        const SPF_UI_API* uiAPI = nullptr;

        HWND gameWindow = nullptr;

        bool showWindow = true;
        bool showWebView = false;
        bool webViewFocus = false;
        bool uiVisible = true;
        bool webViewUnavailableLogged = false;
        bool webViewInitLogged = false;
        uint64_t lastTelemetryPushTickMs = 0;
    };

    extern const char* PLUGIN_NAME;
    extern PluginContext g_ctx;

    void BuildManifest(SPF_Manifest_Builder_Handle* h, const SPF_Manifest_Builder_API* api);
    void OnLoad(const SPF_Load_API* load_api);
    void OnActivated(const SPF_Core_API* core_api);
    void OnUpdate();
    void OnUnload();
    void OnRegisterUI(SPF_UI_API* ui_api);
    void OnSettingChanged(SPF_Config_Handle* config_handle, const char* keyPath);
}