#include "../Internal/PWEOverlayInternal.hpp"

#include <cstring>
#include <string>

#include "../PWEWebView.hpp"

namespace PWE::Internal {
    const char* kDefaultWebViewUrl = "https://pwe.overlay/index.html";
    const char* kWebViewUrlConfigKey = "settings.webview.url";

    namespace {
        struct WindowSearchData {
            DWORD processId = 0;
            HWND window = nullptr;
        };

        DWORD g_lastActionTick = 0;

        BOOL CALLBACK FindProcessMainWindowProc(HWND hwnd, LPARAM lParam) {
            auto* data = reinterpret_cast<WindowSearchData*>(lParam);
            if (!data || !IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER) != nullptr) return TRUE;

            DWORD windowProcessId = 0;
            GetWindowThreadProcessId(hwnd, &windowProcessId);
            if (windowProcessId != data->processId) return TRUE;

            data->window = hwnd;
            return FALSE;
        }

        bool ShouldAcceptAction() {
            const DWORD now = GetTickCount();
            if (now - g_lastActionTick < 120) return false;
            g_lastActionTick = now;
            return true;
        }

        bool IsKeyJustPressed(int virtualKey, bool& wasDown) {
            const bool isDown = (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
            const bool justPressed = isDown && !wasDown;
            wasDown = isDown;
            return justPressed;
        }

        bool ShouldForceLocalUiUrl(const char* url) {
            if (!url || !url[0]) return true;
            const std::string value(url);
            return value.rfind("https://pwe.overlay/", 0) != 0;
        }
    }  // namespace

    void ApplyFocusState() {
        static HWND lastHostWindow = nullptr;
        static bool lastAppliedInteractive = false;
        static bool lastAppliedFocus = false;

        if (!g_ctx.showWebView) g_ctx.webViewFocus = false;

        const HWND currentHostWindow = g_ctx.gameWindow;
        const bool desiredFocus = g_ctx.showWebView && g_ctx.webViewFocus;
        const bool desiredInteractive = desiredFocus;

        if (currentHostWindow != lastHostWindow) {
            lastHostWindow = currentHostWindow;
            lastAppliedInteractive = !desiredInteractive;
            lastAppliedFocus = !desiredFocus;
        }

        if (desiredInteractive != lastAppliedInteractive) {
            PWE::SetWebViewOverlayInteractive(desiredInteractive);
            lastAppliedInteractive = desiredInteractive;
        }

        if (desiredFocus != lastAppliedFocus) {
            PWE::SetWebViewOverlayFocus(desiredFocus);
            lastAppliedFocus = desiredFocus;
        }
    }

    HWND ResolveGameWindow() {
        if (g_ctx.gameWindow && IsWindow(g_ctx.gameWindow)) return g_ctx.gameWindow;
        char gameName[128] = {};

        if (g_ctx.coreAPI && g_ctx.coreAPI->environment && g_ctx.environmentHandle) {
            g_ctx.coreAPI->environment->Env_GetGameName(g_ctx.environmentHandle, gameName, static_cast<int>(sizeof(gameName)));
            if (gameName[0]) g_ctx.gameWindow = FindWindowA(nullptr, gameName);
        }

        if (!g_ctx.gameWindow) {
            HWND foreground = GetForegroundWindow();
            if (foreground && IsWindow(foreground)) {
                DWORD foregroundPid = 0;
                GetWindowThreadProcessId(foreground, &foregroundPid);
                if (foregroundPid == GetCurrentProcessId()) g_ctx.gameWindow = foreground;
            }
        }

        if (!g_ctx.gameWindow) g_ctx.gameWindow = FindWindowW(nullptr, L"Euro Truck Simulator 2");
        if (!g_ctx.gameWindow) g_ctx.gameWindow = FindWindowW(nullptr, L"American Truck Simulator");

        if (!g_ctx.gameWindow) {
            WindowSearchData searchData{};
            searchData.processId = GetCurrentProcessId();
            EnumWindows(&FindProcessMainWindowProc, reinterpret_cast<LPARAM>(&searchData));
            if (searchData.window && IsWindow(searchData.window)) g_ctx.gameWindow = searchData.window;
        }

        if (!g_ctx.gameWindow) g_ctx.gameWindow = GetActiveWindow();
        return g_ctx.gameWindow;
    }

    void ToggleWebViewVisibility() {
        if (!ShouldAcceptAction()) return;
        g_ctx.showWebView = !g_ctx.showWebView;
        if (g_ctx.showWebView) {
            g_ctx.webViewFocus = true;
            PWE::NavigateWebViewOverlay();
        } else {
            g_ctx.webViewFocus = false;
            PWE::SetWebViewOverlayVisible(false);
            PWE::SetWebViewOverlayInteractive(false);
            PWE::SetWebViewOverlayFocus(false);
        }
    }

    void ToggleWebViewFocus() {
        if (!ShouldAcceptAction()) return;
        if (!g_ctx.showWebView) return;
        g_ctx.webViewFocus = !g_ctx.webViewFocus;
        ApplyFocusState();
    }

    void CloseWebView() {
        if (!ShouldAcceptAction()) return;
        if (!g_ctx.showWebView) return;
        g_ctx.showWebView = false;
        g_ctx.webViewFocus = false;
        PWE::SetWebViewOverlayVisible(false);
        PWE::SetWebViewOverlayInteractive(false);
        PWE::SetWebViewOverlayFocus(false);
    }

    void ToggleAction() {
        PWE::Internal::isAction = !PWE::Internal::isAction;
    }

    void ProcessHotkeys() {
        static bool homeWasDown = false;
        if (IsKeyJustPressed(VK_HOME, homeWasDown)) ToggleWebViewFocus();
    }
}  // namespace PWE::Internal
