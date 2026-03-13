<script setup lang="ts">
    import { computed, ref } from 'vue';
    import { useRouter } from 'vue-router';
    import { useTelemetryStore } from '@stores/telemetry';
    import { useLayoutStore } from '@stores/layout';
    import { useSettingsStore } from '@stores/settings';
    import { usePluginBridgeStore } from '@stores/pluginBridge';
    import { Locale } from '@composables/useLanguage';

    import { faArrowLeft, faLayerGroup, faPenToSquare, faGear } from '@fortawesome/free-solid-svg-icons';

    const router = useRouter();
    const telemetry = useTelemetryStore();
    const layout = useLayoutStore();
    const settings = useSettingsStore();
    const bridge = usePluginBridgeStore();
    const moneyDelta = ref(1000);
    const secretClicks = ref(0);
    const showMoneyTools = ref(false);

    const pauseLabel = computed(() => telemetry.data.gamePaused ? 'PAUSED' : '');
    const speedUnitLabel = computed(() => settings.settings.speedUnit === 'mph' ? 'mph' : 'km/h');
    const moneyLabel = computed(() => {
        const value = Number(telemetry.data.money ?? 0);
        if (!Number.isFinite(value)) return '0';
        return new Intl.NumberFormat('pl-PL').format(Math.round(value));
    });

    function goBack() {
        router.push('/');
    }

    function toggleEditMode() {
        router.push('/');
        layout.toggleSettingsMenu();
    }

    function revealSecretTools() {
        secretClicks.value += 1;
        if (secretClicks.value >= 6) showMoneyTools.value = true;
    }

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
</script>

<template>
    <div class="pause-page select-none">
        <div class="pause-page__bg" />
        <div class="pause-page__container">
            <div class="pause-page__header">
                <div class="pause-page__state">{{ pauseLabel }}</div>
                <button class="pause-page__back" @click="goBack">
                    <FontAwesomeIcon :icon="faArrowLeft" />
                    <span>{{ Locale('pauseMenu.back') }}</span>
                </button>
            </div>
            <!--  -->
            <div class="pause-page__title">
                <h1>PWE</h1>
            </div>
            <!--  -->
            <div class="pause-page__grid">
                <section class="pause-card">
                    <div class="pause-card__head">
                        <FontAwesomeIcon :icon="faLayerGroup" />
                        <span>{{ Locale('pauseMenu.quickStatus') }}</span>
                    </div>
                    <div class="pause-card__rows">
                        <div><span>{{ Locale('settings.sdk.connected') }}</span><strong>{{ telemetry.connected ? Locale('settings.sdk.connected') : Locale('settings.sdk.disconnected') }}</strong></div>
                        <div><span>{{ Locale('settings.layout.editMode') }}</span><strong>{{ layout.editMode ? Locale('settings.layout.on') : Locale('settings.layout.off') }}</strong></div>
                        <div><span>{{ Locale('settings.view.speedUnit') }}</span><strong>{{ speedUnitLabel }}</strong></div>
                        <div><span>{{ Locale('pauseMenu.speed') }}</span><strong>{{ Math.round(telemetry.data.speed) }} {{ speedUnitLabel }}</strong></div>
                    </div>
                </section>
                <!--  -->
                <section class="pause-card">
                    <div class="pause-card__head">
                        <FontAwesomeIcon :icon="faPenToSquare" />
                        <span>{{ Locale('pauseMenu.actions') }}</span>
                    </div>
                    <div class="pause-card__actions">
                        <button @click="toggleEditMode">
                            {{ layout.editMode ? Locale('pauseMenu.finishEdit') : Locale('pauseMenu.editLayout') }}
                        </button>
                    </div>
                </section>
                <!--  -->
                <section class="pause-card" v-if="showMoneyTools">
                    <div class="pause-card__head">
                        <FontAwesomeIcon :icon="faGear" />
                        <span>{{ Locale('pauseMenu.cheat') }}</span>
                    </div>
                    <div class="pause-card__actions">
                        <input
                            v-model.number="moneyDelta"
                            type="number"
                            min="1"
                            step="100"
                            class="pause-card__input"
                        >
                        <button :disabled="!bridge.available" @click="addMoney">{{ Locale('pauseMenu.addMoney') }}</button>
                        <button :disabled="!bridge.available" @click="removeMoney">{{ Locale('pauseMenu.removeMoney') }}</button>
                    </div>
                    <p class="pause-card__text" v-if="bridge.lastError">{{ Locale('pauseMenu.error') }}: {{ bridge.lastError }}</p>
                </section>
            </div>
        </div>
        <div class="pause-page__money-corner">{{ moneyLabel }}</div>
        <button
            class="pause-page__secret-corner"
            title="."
            @click="revealSecretTools"
        />
    </div>
</template>

<style scoped>
    .pause-page {
        position: fixed;
        inset: 0;
        color: #e5e7eb;
        pointer-events: auto;
    }

    .pause-page__bg {
        position: absolute;
        inset: 0;
        background:
            radial-gradient(circle at 15% 20%, rgba(94, 212, 255, 0.17), transparent 40%),
            radial-gradient(circle at 85% 80%, rgba(255, 79, 73, 0.14), transparent 36%),
            linear-gradient(160deg, rgba(11, 17, 27, 0.97), rgba(5, 9, 16, 0.96));
        backdrop-filter: blur(8px);
    }

    .pause-page__container {
        position: relative;
        z-index: 1;
        max-width: 980px;
        margin: 0 auto;
        padding: 28px 24px;
    }

    .pause-page__money-corner {
        position: fixed;
        right: 8px;
        bottom: 6px;
        z-index: 5;
        font-size: 11px;
        color: rgba(230, 237, 247, 0.85);
        letter-spacing: 0.04em;
        background: rgba(6, 10, 16, 0.6);
        border: 1px solid rgba(148, 163, 184, 0.4);
        border-radius: 9999px;
        padding: 2px 8px;
    }

    .pause-page__secret-corner {
        position: fixed;
        right: 0;
        top: 0;
        width: 28px;
        height: 28px;
        z-index: 6;
        opacity: 0.02;
        border: 0;
        background: #fff;
        cursor: default;
    }

    .pause-page__header {
        display: flex;
        align-items: center;
        justify-content: space-between;
        margin-bottom: 20px;
    }

    .pause-page__state {
        font-size: 12px;
        text-transform: uppercase;
        letter-spacing: 0.08em;
        color: #5ed4ff;
    }

    .pause-page__back {
        display: inline-flex;
        align-items: center;
        gap: 8px;
        border: 1px solid rgba(148, 163, 184, 0.45);
        background: rgba(10, 14, 22, 0.78);
        color: #cbd5e1;
        border-radius: 9999px;
        padding: 8px 12px;
        cursor: pointer;
        font-family: inherit;
        font-size: 12px;
    }

    .pause-page__back:hover {
        border-color: rgba(94, 212, 255, 0.75);
        color: #67e8f9;
    }

    .pause-page__title h1 {
        margin: 0;
        font-size: 32px;
        line-height: 1.1;
        color: #e6edf7;
    }

    .pause-page__title p {
        margin: 8px 0 0;
        color: #8f9db1;
        font-size: 14px;
    }

    .pause-page__grid {
        margin-top: 22px;
        display: grid;
        grid-template-columns: repeat(12, minmax(0, 1fr));
        gap: 14px;
    }

    .pause-card {
        grid-column: span 12;
        border: 1px solid rgba(148, 163, 184, 0.32);
        border-radius: 14px;
        background: linear-gradient(180deg, rgba(16, 23, 36, 0.86), rgba(9, 13, 20, 0.84));
        backdrop-filter: blur(10px);
        padding: 14px;
    }

    .pause-card__head {
        display: inline-flex;
        align-items: center;
        gap: 8px;
        font-size: 12px;
        text-transform: uppercase;
        letter-spacing: 0.08em;
        color: #cbd5e1;
        margin-bottom: 10px;
    }

    .pause-card__rows {
        display: grid;
        gap: 8px;
    }

    .pause-card__rows div {
        display: flex;
        justify-content: space-between;
        gap: 8px;
        border-bottom: 1px solid rgba(71, 85, 105, 0.45);
        padding-bottom: 6px;
        font-size: 13px;
        color: #8f9db1;
    }

    .pause-card__rows strong {
        color: #e6edf7;
        font-weight: 700;
    }

    .pause-card__actions {
        display: flex;
        flex-wrap: wrap;
        gap: 8px;
    }

    .pause-card__actions button {
        border: 1px solid rgba(148, 163, 184, 0.36);
        background: rgba(6, 10, 16, 0.74);
        color: #dce6f4;
        border-radius: 10px;
        padding: 8px 12px;
        cursor: pointer;
        font-family: inherit;
        font-size: 12px;
    }

    .pause-card__actions button:hover {
        border-color: rgba(94, 212, 255, 0.78);
        color: #67e8f9;
    }

    .pause-card__input {
        width: 120px;
        border: 1px solid rgba(148, 163, 184, 0.36);
        background: rgba(6, 10, 16, 0.74);
        color: #e6edf7;
        border-radius: 10px;
        padding: 8px 10px;
        font-family: inherit;
        font-size: 12px;
    }

    .pause-card__text {
        margin: 0;
        color: #8f9db1;
        font-size: 13px;
        line-height: 1.5;
    }

    @media (min-width: 900px) {
        .pause-card:nth-child(1) { grid-column: span 5; }
        .pause-card:nth-child(2) { grid-column: span 4; }
        .pause-card:nth-child(3) { grid-column: span 3; }
    }
</style>