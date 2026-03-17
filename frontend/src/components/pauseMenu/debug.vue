<script setup lang="ts">
    import { computed, ref } from 'vue';
    import { usePluginBridgeStore } from '@stores/pluginBridge';
    import { useMiscStore } from '@stores/misc';
    import { useLayoutStore } from '@stores/layout';
    import { Locale } from '@composables/useLanguage';
    import { faBug, faSuitcase } from '@fortawesome/free-solid-svg-icons';

    import type { TelemetryData } from '@interfaces/telemetry';

    const moneyDelta = ref(1000);

    const layout = useLayoutStore();
    const bridge = usePluginBridgeStore();
    const misc = useMiscStore();

    const disableEconomyTools = computed(() => !bridge.available || misc.miscSettings.isMultiplayer);

    function sanitizeMoneyDelta() {
        if (!Number.isFinite(moneyDelta.value) || moneyDelta.value < 1) {
            moneyDelta.value = 1;
            return;
        }
        moneyDelta.value = Math.round(moneyDelta.value);
    }

    function addMoney() {
        sanitizeMoneyDelta();
        bridge.sendCommand('economy/add_money', { amount: moneyDelta.value });
    }

    function removeMoney() {
        sanitizeMoneyDelta();
        bridge.sendCommand('economy/remove_money', { amount: moneyDelta.value });
    }

    const props = defineProps<{
        activeTab: string;
        telemetry: TelemetryData;
        connected: boolean;
    }>();
</script>

<template>
    <section v-show="activeTab === 'debug'" key="debug" class="pm2__grid">
        <div class="pm2__card hud-glass">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faBug" />
                <span>{{ Locale('pauseMenu2.debug.title') }}</span>
            </div>

            <div class="pm2__kv pm2__kv--cols">
                <div>
                    <span>{{ Locale('pauseMenu2.debug.bridge') }}</span>
                    <strong>{{ bridge.available ? Locale('pauseMenu2.debug.available') : Locale('pauseMenu2.debug.unavailable') }}</strong>
                </div>

                <div>
                    <span>{{ Locale('pauseMenu2.debug.webViewFocus') }}</span>
                    <strong>{{ misc.miscSettings.webViewFocus ? Locale('pauseMenu2.debug.on') : Locale('pauseMenu2.debug.off') }}</strong>
                </div>

                <div>
                    <span>{{ Locale('pauseMenu2.debug.editMode') }}</span>
                    <strong>{{ layout.editMode ? Locale('pauseMenu2.debug.on') : Locale('pauseMenu2.debug.off') }}</strong>
                </div>

                <div>
                    <span>{{ Locale('pauseMenu2.debug.multiplayer') }}</span>
                    <strong>{{ misc.miscSettings.isMultiplayer ? Locale('pauseMenu2.debug.on') : Locale('pauseMenu2.debug.off') }}</strong>
                </div>

                <div>
                    <span>{{ Locale('pauseMenu2.debug.money') }}</span>
                    <strong>{{ telemetry.money }}</strong>
                </div>
            </div>
        </div>

        <div class="pm2__card hud-glass">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faSuitcase" />
                <span>{{ Locale('pauseMenu2.debug.devTools') }}</span>
            </div>
            <div class="pm2__debug-row">
                <input class="pm2__input" type="number" v-model="moneyDelta" min="1" step="1" />
                <button class="pm2__btn pm2__btn--ghost" :disabled="disableEconomyTools" @click="addMoney">
                    <span>{{ Locale('pauseMenu2.debug.addMoney') }}</span>
                </button>
                <button class="pm2__btn pm2__btn--ghost" :disabled="disableEconomyTools" @click="removeMoney">
                    <span>{{ Locale('pauseMenu2.debug.removeMoney') }}</span>
                </button>
            </div>

            <div class="pm2__hint" v-if="misc.miscSettings.isMultiplayer">{{ Locale('pauseMenu2.multiplayerDisabled') }}</div>
            <div class="pm2__hint pm2__hint--bad" v-if="bridge.lastError">{{ Locale('pauseMenu2.error') }}: {{ bridge.lastError }}</div>
        </div>
    </section>
</template>