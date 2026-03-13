#include "PWEWebView.hpp"

#include <objbase.h>
#include <cstring>
#include <cwchar>
#include <string>

#if __has_include(<WebView2.h>)
    #define HAS_WEBVIEW2 1
    #include <WebView2.h>
    #include <wrl.h>
    #include "PWEOverlay.hpp"
#else
    #define HAS_WEBVIEW2 0
#endif

#if HAS_WEBVIEW2
    using Microsoft::WRL::Callback;
    using Microsoft::WRL::ComPtr;

    namespace {
        HWND g_hostWindow = nullptr;
        HWND g_webViewWindow = nullptr;

        bool g_requestedVisible = false;
        bool g_interactive = false;
        bool g_createStarted = false;
        bool g_comInitialized = false;

        HMODULE g_webView2Loader = nullptr;

        ComPtr<ICoreWebView2Environment> g_environment;
        ComPtr<ICoreWebView2Controller> g_controller;
        ComPtr<ICoreWebView2> g_webView;
        EventRegistrationToken g_messageToken = {};
        EventRegistrationToken g_navigationToken = {};
        PWE::WebViewMessageCallback g_messageCallback = nullptr;

        bool g_messageTokenSet = false;
        bool g_navigationTokenSet = false;
        bool g_localFrontendMapped = false;

        BOOL CALLBACK FindWebViewWindowProc(HWND window, LPARAM lParam) {
            wchar_t className[256] = {};
            if (GetClassNameW(window, className, static_cast<int>(sizeof(className) / sizeof(className[0]))) > 0) {
                if (wcsstr(className, L"Chrome_WidgetWin") != nullptr || wcsstr(className, L"WebView") != nullptr) {
                    *reinterpret_cast<HWND*>(lParam) = window;
                    return FALSE;
                }
            }
            return TRUE;
        }

        void TryBindWebViewWindow() {
            if (!g_hostWindow || g_webViewWindow) return;
            HWND found = nullptr;
            EnumChildWindows(g_hostWindow, &FindWebViewWindowProc, reinterpret_cast<LPARAM>(&found));

            if (found) g_webViewWindow = found;
        }

        bool NavigateUrlInternal() {
            if (!g_webView) return false;
            const char* url = "https://pwe.overlay/index.html";

            const int requiredSize = MultiByteToWideChar(CP_UTF8, 0, url, -1, nullptr, 0);
            if (requiredSize <= 1) return false;

            std::wstring wideUrl(static_cast<size_t>(requiredSize), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, url, -1, wideUrl.data(), requiredSize);
            return SUCCEEDED(g_webView->Navigate(wideUrl.c_str()));
        }

        std::string WideToUtf8(const wchar_t* wide) {
            if (!wide || !wide[0]) return {};

            const int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
            if (requiredSize <= 1) return {};

            std::string utf8(static_cast<size_t>(requiredSize - 1), '\0');
            WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8.data(), requiredSize - 1, nullptr, nullptr);
            return utf8;
        }

        std::wstring ResolvePluginDirectory() {
            HMODULE moduleHandle = nullptr;
            if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                reinterpret_cast<LPCWSTR>(&PWE::InitializeWebViewOverlay), &moduleHandle))
            {
                return {};
            }

            wchar_t modulePath[MAX_PATH] = {};
            if (!GetModuleFileNameW(moduleHandle, modulePath, static_cast<DWORD>(sizeof(modulePath) / sizeof(modulePath[0])))) {
                return {};
            }

            std::wstring path(modulePath);
            const size_t slashPos = path.find_last_of(L"\\/");
            if (slashPos == std::wstring::npos) return {};

            return path.substr(0, slashPos);
        }

        bool TryMapLocalFrontendFolder() {
            if (!g_webView) return false;

            std::wstring pluginDir = ResolvePluginDirectory();
            if (pluginDir.empty()) return false;

            std::wstring uiDir = pluginDir + L"\\ui";
            const DWORD attrs = GetFileAttributesW(uiDir.c_str());
            if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                if (PWE::g_ctx.loggerHandle && PWE::g_ctx.loadAPI && PWE::g_ctx.loadAPI->logger) {
                    std::string utf8Path = WideToUtf8(uiDir.c_str());
                    char msg[512] = {};
                    std::snprintf(msg, sizeof(msg), "PWEOverlay: ui folder check failed: %s (attrs=%lu)", utf8Path.c_str(), attrs);
                    PWE::g_ctx.loadAPI->logger->Log(PWE::g_ctx.loggerHandle, SPF_LOG_WARN, msg);
                }
                return false;
            }

            ComPtr<ICoreWebView2_3> webView3;
            if (FAILED(g_webView.As(&webView3)) || !webView3) {
                if (PWE::g_ctx.loggerHandle && PWE::g_ctx.loadAPI && PWE::g_ctx.loadAPI->logger) {
                    PWE::g_ctx.loadAPI->logger->Log(PWE::g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: ICoreWebView2_3 not available");
                }
                return false;
            }

            HRESULT hr = webView3->SetVirtualHostNameToFolderMapping(L"pwe.overlay", uiDir.c_str(), COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_ALLOW);
            return SUCCEEDED(hr);
        }

        bool IsLocalFrontendUrl(const char* url) {
            return url && std::strstr(url, "pwe.overlay") != nullptr;
        }

        const char* ResolveStartupUrl(const char* requestedUrl) {
            if (requestedUrl && requestedUrl[0] && IsLocalFrontendUrl(requestedUrl)) return requestedUrl;
            return "https://pwe.overlay/index.html";
        }

        void NavigateMissingUiErrorPage() {
            if (!g_webView) return;
            g_webView->NavigateToString(
                LR"HTML(
                    <html>
                        <body style="margin:0;background:#0b1020;color:#e5e7eb;font-family:Consolas,monospace;padding:20px;">
                            <h1>Missing UI</h1>
                            <p>Unable to load UI. Please ensure the UI folder is present in the plugin directory.</p>
                        </body>
                    </html>
                )HTML"
            );
        }

        void ApplyInteractivityInternal() {
            TryBindWebViewWindow();
            if (!g_webViewWindow) return;

            LONG_PTR exStyle = GetWindowLongPtrW(g_webViewWindow, GWL_EXSTYLE);
            if (g_interactive) {
                exStyle &= ~WS_EX_TRANSPARENT;
            } else {
                exStyle |= WS_EX_TRANSPARENT;
            }
            SetWindowLongPtrW(g_webViewWindow, GWL_EXSTYLE, exStyle);
            SetWindowPos(g_webViewWindow, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }

        void UpdateBoundsInternal() {
            if (!g_hostWindow || !g_controller) return;
            RECT rect = {};
            if (GetClientRect(g_hostWindow, &rect)) g_controller->put_Bounds(rect);
        }

        void UpdateVisibilityInternal() {
            if (!g_controller) return;
            g_controller->put_IsVisible(g_requestedVisible ? TRUE : FALSE);
            if (!g_requestedVisible) g_interactive = false;
            ApplyInteractivityInternal();
        }
    } //namespace
#endif

namespace PWE {
    bool InitializeWebViewOverlay(HWND hostWindow) {
        const char* initialUrl = "https://pwe.overlay/";
        if (!hostWindow) return false;

        #if HAS_WEBVIEW2
            g_hostWindow = hostWindow;
            if (g_controller) return true;
            if (g_createStarted) return false;

            g_createStarted = true;
            if (!g_comInitialized) {
                const HRESULT comHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
                if (SUCCEEDED(comHr)) g_comInitialized = true;
            }

            if (!g_webView2Loader) {
                g_webView2Loader = LoadLibraryW(L"WebView2Loader.dll");
                if (!g_webView2Loader) {
                    g_createStarted = false;
                    return false;
                }
            }

            using CreateEnvironmentFn = HRESULT(STDAPICALLTYPE*)(PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*,
                                                       ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);
            const auto createEnvironment = reinterpret_cast<CreateEnvironmentFn>(GetProcAddress(g_webView2Loader, "CreateCoreWebView2EnvironmentWithOptions"));
            if (!createEnvironment) {
                g_createStarted = false;
                return false;
            }

            const HRESULT hr = createEnvironment(nullptr, nullptr, nullptr,
                Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([initialUrl](HRESULT envResult, ICoreWebView2Environment* environment) -> HRESULT
            {
                if (FAILED(envResult) || !environment) {
                    g_createStarted = false;
                    return S_OK;
                }

                g_environment = environment;
                return environment->CreateCoreWebView2Controller(g_hostWindow,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>([initialUrl](HRESULT controllerResult, ICoreWebView2Controller* controller) -> HRESULT
                {
                    if (FAILED(controllerResult) || !controller) {
                        g_createStarted = false;
                        return S_OK;
                    }

                    g_controller = controller;
                    g_controller->get_CoreWebView2(&g_webView);
                    ComPtr<ICoreWebView2Controller2> controller2;
                    if (SUCCEEDED(g_controller.As(&controller2))) {
                        COREWEBVIEW2_COLOR transparentColor = {0, 0, 0, 0};
                        controller2->put_DefaultBackgroundColor(transparentColor);
                    }

                    if (g_webView) {
                        ComPtr<ICoreWebView2Settings> settings;
                        if (SUCCEEDED(g_webView->get_Settings(&settings)) && settings) {
                            settings->put_AreDefaultContextMenusEnabled(FALSE);
                        }

                        g_webView->add_NavigationCompleted(
                            Callback<ICoreWebView2NavigationCompletedEventHandler>([](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
                        {
                            if (!sender || !args) return S_OK;

                            BOOL isSuccess = FALSE;
                            args->get_IsSuccess(&isSuccess);
                            if (isSuccess) return S_OK;

                            COREWEBVIEW2_WEB_ERROR_STATUS errorStatus = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
                            args->get_WebErrorStatus(&errorStatus);

                            LPWSTR source = nullptr;
                            std::string sourceUtf8;
                            if (SUCCEEDED(sender->get_Source(&source)) && source) {
                                sourceUtf8 = WideToUtf8(source);
                                CoTaskMemFree(source);
                            }

                            if (PWE::g_ctx.loggerHandle && PWE::g_ctx.loadAPI && PWE::g_ctx.loadAPI->logger) {
                                char msg[512] = {};
                                std::snprintf(msg, sizeof(msg),
                                    "PWEOverlay: navigation failed (status=%d, source=%s)",
                                    static_cast<int>(errorStatus),
                                    sourceUtf8.empty() ? "<empty>" : sourceUtf8.c_str()
                                );
                                PWE::g_ctx.loadAPI->logger->Log(
                                PWE::g_ctx.loggerHandle, SPF_LOG_WARN, msg);
                            }

                            if (errorStatus == COREWEBVIEW2_WEB_ERROR_STATUS_CONNECTION_ABORTED && sourceUtf8 == "about:blank") {
                                return S_OK;
                            }

                            if (g_localFrontendMapped && (sourceUtf8.empty() || sourceUtf8.find("pwe.overlay") == std::string::npos)) {
                                sender->Navigate(L"https://pwe.overlay/");
                            }
                            return S_OK;
                        }).Get(), &g_navigationToken);

                        g_navigationTokenSet = true;
                        g_webView->add_WebMessageReceived(
                            Callback<ICoreWebView2WebMessageReceivedEventHandler>([](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                        {
                            (void)sender;
                            if (!args || !g_messageCallback) return S_OK;

                            LPWSTR message = nullptr;
                            std::string utf8;

                            if (SUCCEEDED(args->get_WebMessageAsJson(&message)) && message) {
                                utf8 = WideToUtf8(message);
                                CoTaskMemFree(message);
                                message = nullptr;
                            } else if (SUCCEEDED(args->TryGetWebMessageAsString(&message)) && message) {
                                utf8 = WideToUtf8(message);
                                CoTaskMemFree(message);
                                message = nullptr;
                            }

                            if (!utf8.empty()) g_messageCallback(utf8.c_str());
                            return S_OK;
                        }).Get(), &g_messageToken);

                        g_messageTokenSet = true;
                        g_webView->AddScriptToExecuteOnDocumentCreated(
                            LR"JS(
                                (() => {
                                    const css = `
                                        html, body, #root, #app {
                                            background: transparent !important;
                                        }
                                    `;

                                    const style = document.createElement('style');
                                    style.id = '__spf_overlay_transparency';
                                    style.textContent = css;

                                    const ensureStyle = () => {
                                        if (!document.getElementById(style.id)) {
                                            (document.head || document.documentElement).appendChild(style.cloneNode(true));
                                        }
                                    };

                                    if (document.readyState === 'loading') {
                                        document.addEventListener('DOMContentLoaded', ensureStyle, { once: true });
                                    } else {
                                        ensureStyle();
                                    }
                                })();
                            )JS",
                        nullptr);
                    }

                    g_localFrontendMapped = TryMapLocalFrontendFolder();
                    const char* startupUrl = ResolveStartupUrl(initialUrl);
                    bool navigated = false;

                    if (g_localFrontendMapped) navigated = NavigateUrlInternal();
                    if (!navigated && g_localFrontendMapped) navigated = NavigateUrlInternal();

                    if (!navigated && IsLocalFrontendUrl(startupUrl) && !g_localFrontendMapped &&
                        PWE::g_ctx.loggerHandle && PWE::g_ctx.loadAPI &&
                        PWE::g_ctx.loadAPI->logger)
                    {
                        PWE::g_ctx.loadAPI->logger->Log(PWE::g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: local frontend not found (expected ./ui beside plugin DLL).");
                        NavigateMissingUiErrorPage();
                        navigated = true;
                    }
                    if (!navigated) {
                        g_webView->NavigateToString(L"<html style='background:transparent'><body style='margin:0;background:transparent;'></body></html>");
                    }

                    UpdateBoundsInternal();
                    UpdateVisibilityInternal();
                    g_createStarted = false;
                    return S_OK;
                }).Get());
            }).Get());

            if (FAILED(hr)) {
                g_createStarted = false;
                return false;
            }
            return false;
        #else
            (void)hostWindow;
            (void)initialUrl;
            return false;
        #endif
    }

    void SetWebViewOverlayVisible(bool visible) {
        #if HAS_WEBVIEW2
            g_requestedVisible = visible;
            UpdateVisibilityInternal();
        #else
            (void)visible;
        #endif
    }

    void SetWebViewOverlayInteractive(bool interactive) {
        #if HAS_WEBVIEW2
            g_interactive = interactive && g_requestedVisible;
            ApplyInteractivityInternal();
        #else
            (void)interactive;
        #endif
    }

    void SetWebViewOverlayFocus(bool focusWebView) {
        #if HAS_WEBVIEW2
            if (!g_hostWindow || !IsWindow(g_hostWindow)) return;

            if (focusWebView && g_requestedVisible) {
                TryBindWebViewWindow();
                if (g_controller) g_controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);

                if (g_webViewWindow && IsWindow(g_webViewWindow)) {
                    SetForegroundWindow(g_hostWindow);
                    SetFocus(g_webViewWindow);
                }
                return;
            }

            SetForegroundWindow(g_hostWindow);
            SetActiveWindow(g_hostWindow);
            SetFocus(g_hostWindow);
        #else
            (void)focusWebView;
        #endif
    }

    bool NavigateWebViewOverlay() {
        #if HAS_WEBVIEW2
            return NavigateUrlInternal();
        #else
            return false;
        #endif
    }

    bool PostWebViewOverlayMessageJson(const char* json) {
        #if HAS_WEBVIEW2
            if (!g_webView || !json || !json[0]) return false;

            const int requiredSize = MultiByteToWideChar(CP_UTF8, 0, json, -1, nullptr, 0);
            if (requiredSize <= 1) return false;

            std::wstring wideJson(static_cast<size_t>(requiredSize), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, json, -1, wideJson.data(), requiredSize);
            return SUCCEEDED(g_webView->PostWebMessageAsJson(wideJson.c_str()));
        #else
            (void)json;
            return false;
        #endif
    }

    void SetWebViewOverlayMessageCallback(WebViewMessageCallback callback) {
        #if HAS_WEBVIEW2
            g_messageCallback = callback;
        #else
            (void)callback;
        #endif
    }

    void UpdateWebViewOverlayBounds() {
        #if HAS_WEBVIEW2
            UpdateBoundsInternal();
            ApplyInteractivityInternal();
        #endif
    }

    void ShutdownWebViewOverlay() {
        #if HAS_WEBVIEW2
            if (g_webView && g_messageTokenSet) {
                g_webView->remove_WebMessageReceived(g_messageToken);
                g_messageTokenSet = false;
            }

            if (g_webView && g_navigationTokenSet) {
                g_webView->remove_NavigationCompleted(g_navigationToken);
                g_navigationTokenSet = false;
            }

            if (g_controller) {
                g_controller->put_IsVisible(FALSE);
                g_controller->Close();
            }

            g_webView.Reset();
            g_controller.Reset();
            g_environment.Reset();
            g_webViewWindow = nullptr;

            if (g_webView2Loader) {
                FreeLibrary(g_webView2Loader);
                g_webView2Loader = nullptr;
            }

            if (g_comInitialized) {
                CoUninitialize();
                g_comInitialized = false;
            }

            g_hostWindow = nullptr;
            g_createStarted = false;
            g_requestedVisible = false;
            g_interactive = false;
            g_localFrontendMapped = false;
        #endif
    }

    bool IsWebViewOverlayAvailable() {
        #if HAS_WEBVIEW2
            return true;
        #else
            return false;
        #endif
    }
}  // namespace PWE