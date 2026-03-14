#pragma once
#include <windows.h>

namespace PWE {
    using WebViewMessageCallback = void(*)(const char* jsonMessage);

    bool InitializeWebViewOverlay(HWND hostWindow);

    void SetWebViewOverlayVisible(bool visible);
    void SetWebViewOverlayInteractive(bool interactive);
    void SetWebViewOverlayFocus(bool focusWebView);
    void SetWebViewOverlayMessageCallback(WebViewMessageCallback callback);
    void UpdateWebViewOverlayBounds();
    void TickWebViewOverlayFocusGuard();
    void ShutdownWebViewOverlay();

    bool PostWebViewOverlayMessageJson(const char* json);
    bool NavigateWebViewOverlay();
    bool IsWebViewOverlayAvailable();
}
