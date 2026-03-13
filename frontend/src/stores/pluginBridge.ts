import { computed, ref } from 'vue';
import { defineStore } from 'pinia';
import type { PluginResponsePayload, PluginSender, PluginCommandPayload } from '@interfaces/plugin';

const usePluginBridgeStore = defineStore('pluginBridge', () => {
    const sender = ref<PluginSender>(null);
    const lastResponse = ref<PluginResponsePayload | null>(null);
    const lastError = ref<string>('');
    const requestCounter = ref(0);

    const available = computed(() => Boolean(sender.value));

    function setSender(nextSender: PluginSender) {
        sender.value = nextSender;
    }

    function sendCommand(command: string, payload: PluginCommandPayload = {}) {
        if (!sender.value) {
            lastError.value = 'bridge_unavailable';
            return false;
        }

        requestCounter.value += 1;
        const requestId = `cmd_${Date.now()}_${requestCounter.value}`;
        const sent = sender.value({
            type: 'plugin/command',
            payload: {
                requestId,
                command,
                ...payload,
            },
        });

        if (!sent) lastError.value = 'send_failed';
        return sent;
    }

    function applyResponse(payload: PluginResponsePayload) {
        lastResponse.value = payload;
        if (payload?.ok === false) {
            lastError.value = payload.message || 'plugin_error';
            return;
        }
        lastError.value = '';
    }

    return {
        available,
        lastResponse,
        lastError,
        setSender,
        sendCommand,
        applyResponse,
    };
});

export { usePluginBridgeStore };