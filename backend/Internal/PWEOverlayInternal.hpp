#pragma once

#include <string>

#include "../PWEOverlay.hpp"

namespace PWE::Internal {
    extern const char* kDefaultWebViewUrl;
    extern const char* kWebViewUrlConfigKey;

    void LoadWebViewUrlFromConfig();
    void ApplyFocusState();

    HWND ResolveGameWindow();

    void ToggleWebViewVisibility();
    void ToggleWebViewFocus();
    void CloseWebView();
    void ProcessHotkeys();

    std::string BuildTelemetryMessageJson();

    void PushTelemetryToWebView();
    void UpdateG27LEDs();
    void HandleWebViewMessageJson(const char* jsonMessage);
    void ToggleAction();

    extern bool isAction;
}
