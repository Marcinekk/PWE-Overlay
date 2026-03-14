import { onMounted, onUnmounted, ref } from 'vue';
import { useTelemetryStore } from '@stores/telemetry';
import { usePluginBridgeStore } from '@stores/pluginBridge';

import { useEventsStore, type PendingEvents } from '@stores/events';
import { useMiscStore } from '@stores/misc';
import type { WebViewMessage } from '@interfaces/WebView';

function useTelemetry() {
    const store = useTelemetryStore();
    const miscStore = useMiscStore();
    const pluginBridge = usePluginBridgeStore();
    const reconnectTimer = ref<ReturnType<typeof setTimeout> | null>(null);

    let webViewMessageHandler: ((event: any) => void) | null = null;

    function hasWebViewBridge() {
        const w = window as any;
        return Boolean(w?.chrome?.webview?.addEventListener);
    }

    function applyIncomingWebViewMessage(message: WebViewMessage) {
        if (!message || !message.type) return;
        switch(message.type) {
            case 'telemetry/frame': {
                if(!message.payload) return;
                store.update(message.payload);
                store.setConnected(true);
                break;
            };

            case 'telemetry/state': {
                if(!message.payload) return;
                store.setConnected(Boolean(message.payload?.connected));
                break;
            };

            case 'plugin/response': {
                if(!message.payload) return;
                pluginBridge.applyResponse(message.payload);
                break;
            };

            case 'events/add': {
                if(!message.payload) return;
                useEventsStore().addEvent(message.payload as PendingEvents);
                break;
            }

            case 'misc/versions': {
                if(!message.payload) return;
                miscStore.setGameMismatch(message.payload.game_mismatch as boolean);
                miscStore.setFrameworkMismatch(message.payload.framework_mismatch as boolean);
                break;
            }

            case 'misc/multiplayer': {
                if(!message.payload) return;
                miscStore.setMultiplayer(message.payload.is_multiplayer as boolean);
                break;
            }
        }
    }

    function postWebViewMessage(message: Record<string, unknown>) {
        const w = window as any;
        const webview = w?.chrome?.webview;
        if (!webview || typeof webview.postMessage !== 'function') return false;
        webview.postMessage(message);
        return true;
    }

    function connectWebViewBridge() {
        const w = window as any;
        const webview = w?.chrome?.webview;
        if (!webview || !webview.addEventListener) return false;

        webViewMessageHandler = (event: any) => {
            try {
                const raw = event?.data;
                const message = typeof raw === 'string'
                    ? (JSON.parse(raw) as WebViewMessage)
                    : (raw as WebViewMessage);
                applyIncomingWebViewMessage(message);
            } catch (e) {
                console.warn('[Telemetry] WebView parse error', e);
            }
        };

        webview.addEventListener('message', webViewMessageHandler);
        pluginBridge.setSender(postWebViewMessage);
        return true;
    }

    function connect() {
        if (hasWebViewBridge()) {
            const connected = connectWebViewBridge();
            if (connected) store.setConnected(true);
        }
    }

    function disconnect() {
        if (reconnectTimer.value) clearTimeout(reconnectTimer.value);
        pluginBridge.setSender(null);

        const w = window as any;
        if (webViewMessageHandler && w?.chrome?.webview?.removeEventListener) {
            w.chrome.webview.removeEventListener('message', webViewMessageHandler);
            webViewMessageHandler = null;
        }
    }

    onMounted(() => connect());
    onUnmounted(() => disconnect());
    return { connect, disconnect };
}

export { useTelemetry };