<script setup lang="ts">
    import { computed, onMounted, ref, watch } from 'vue';
    import { useRouter } from 'vue-router';

    import { useTelemetryStore } from '@stores/telemetry';
    import { useLayoutStore } from '@stores/layout';
    import { usePluginBridgeStore } from '@stores/pluginBridge';
    import { useMiscStore } from '@stores/misc';
    import { useSettingsStore } from '@stores/settings';
    import { useTransactionsStore } from '@stores/transactions';
    import { Locale } from '@composables/useLanguage';

    import OverviewPage from '@components/pauseMenu/overview.vue';
    import TruckPage from '@components/pauseMenu/truck.vue';
    import LogbookPage from '@components/pauseMenu/logBook.vue';
    import TransactionsPage from '@components/pauseMenu/transactions.vue';
    import DebugPage from '@components/pauseMenu/debug.vue';

    import {
        faArrowLeft,
        faGaugeHigh,
        faTruck,
        faBug,
        faWrench,
        faPlugCircleBolt,
        faCircleInfo,
        faWallet,
        faBook
    } from '@fortawesome/free-solid-svg-icons';

    type TabId = 'overview' | 'truck' | 'logbook' | 'transactions' | 'debug';

    const router = useRouter();
    const telemetry = useTelemetryStore();
    const layout = useLayoutStore();
    const settings = useSettingsStore();
    const bridge = usePluginBridgeStore();
    const misc = useMiscStore();
    const txStore = useTransactionsStore();

    const activeTab = ref<TabId>('overview');

    const speedUnitLabel = computed(() => (settings.settings.speedUnit === 'mph' ? 'mph' : 'km/h'));
    const isMph = computed(() => settings.settings.speedUnit === 'mph');

    function toDisplaySpeed(kmh: number): number {
        const value = Number(kmh ?? 0);
        if (!Number.isFinite(value)) return 0;
        return isMph.value ? value * 0.621371 : value;
    }

    const speedDisplay = computed(() => Math.round(toDisplaySpeed(telemetry.data.speed)));

    const moneyLabel = computed(() => {
        const value = Number(telemetry.data.money ?? 0);
        if (!Number.isFinite(value)) return '0';
        const locale = settings.settings.language === 'pl' ? 'pl-PL' : 'en-US';
        return new Intl.NumberFormat(locale).format(Math.round(value));
    });

    const gearLabel = computed(() => {
        const g = telemetry.data.gear;
        if (g === 0) return 'N';
        if (g < 0) return 'R';
        return String(g);
    });

    const tabs = computed(() => ([
        { id: 'overview' as const, label: Locale('pauseMenu2.tabs.overview'), icon: faGaugeHigh },
        { id: 'truck' as const, label: Locale('pauseMenu2.tabs.truck'), icon: faTruck },
        { id: 'logbook' as const, label: Locale('pauseMenu2.tabs.logbook'), icon: faBook },
        { id: 'transactions' as const, label: Locale('pauseMenu2.tabs.transactions'), icon: faWallet },
        { id: 'debug' as const, label: Locale('pauseMenu2.tabs.debug'), icon: faBug },
    ]));

    function goBack() {
        router.push('/');
    }

    function toggleEditMode() {
        router.push('/');
        layout.toggleSettingsMenu();
    }

    function refreshInfo() {
        bridge.sendCommand('internal/get_versions', {});
        bridge.sendCommand('internal/get_multiplayer', {});
    }

    function fetchTransactions() {
        txStore.fetchAll();
    }

    watch(activeTab, (tab) => {
        if (tab === 'transactions') fetchTransactions();
    });

    onMounted(() => {
        refreshInfo();
        fetchTransactions();
    });
</script>

<template>
    <div class="pm2 select-none">
        <div class="pm2__bg" />
        <div class="pm2__shell">
            <header class="pm2__header hud-glass-strong">
                <div class="pm2__brand">
                    <div class="pm2__logo">
                        <span>PWE</span>
                    </div>
                    <div class="pm2__title">
                        <div class="pm2__title-top">
                            <span class="pm2__paused" v-if="telemetry.data.gamePaused">{{ Locale('pauseMenu2.paused') }}</span>
                            <span
                                class="pm2__badge"
                                :class="telemetry.connected ? 'pm2__badge--ok' : 'pm2__badge--bad animate__animated animate__pulse animate__infinite'"
                            >
                                <FontAwesomeIcon :icon="faPlugCircleBolt" />
                                <span>{{ telemetry.connected ? Locale('settings.sdk.connected') : Locale('settings.sdk.disconnected') }}</span>
                            </span>
                            <span class="pm2__badge" v-if="misc.miscSettings.isMultiplayer">
                                <FontAwesomeIcon :icon="faCircleInfo" />
                                <span>{{ Locale('pauseMenu2.multiplayer') }}</span>
                            </span>
                        </div>
                        <div class="pm2__title-bottom">
                            <span class="hud-text-muted">{{ Locale('pauseMenu2.money') }}:</span>
                            <strong class="hud-text">{{ moneyLabel }}</strong>
                            <span class="pm2__sep">•</span>
                            <span class="hud-text-muted">{{ Locale('pauseMenu2.gear') }}:</span>
                            <strong class="hud-text">{{ gearLabel }}</strong>
                            <span class="pm2__sep">•</span>
                            <span class="hud-text-muted">{{ Locale('pauseMenu2.speed') }}:</span>
                            <strong class="hud-text">{{ speedDisplay }} {{ speedUnitLabel }}</strong>
                            <span class="pm2__sep">•</span>
                            <span class="hud-text-muted">{{ Locale('pauseMenu2.game') }}:</span>
                            <strong class="hud-text">{{ telemetry.data.gameName || '?' }}</strong>
                        </div>
                    </div>
                </div>
                <div class="pm2__header-actions">
                    <button class="pm2__btn" @click="goBack">
                        <FontAwesomeIcon :icon="faArrowLeft" />
                        <span>{{ Locale('pauseMenu2.back') }}</span>
                    </button>
                    <button class="pm2__btn pm2__btn--primary" @click="toggleEditMode">
                        <FontAwesomeIcon :icon="faWrench" />
                        <span>{{ layout.editMode ? Locale('pauseMenu2.finishEdit') : Locale('pauseMenu2.editLayout') }}</span>
                    </button>
                </div>
            </header>

            <div class="pm2__body">
                <aside class="pm2__nav hud-glass-strong">
                    <button
                        v-for="t in tabs"
                        :key="t.id"
                        class="pm2__nav-item"
                        :class="activeTab === t.id ? 'pm2__nav-item--active' : ''"
                        @click="activeTab = t.id"
                    >
                        <FontAwesomeIcon :icon="t.icon" class="pm2__nav-ico" />
                        <span>{{ t.label }}</span>
                    </button>

                    <!-- <div class="pm2__nav-divider" /> -->
                </aside>

                <main class="pm2__content">
                    <transition
                        mode="out-in"
                        enter-active-class="animate__animated animate__slideInLeft animate__faster"
                        leave-active-class="animate__animated animate__slideOutLeft animate__faster"
                    >
                        <div :key="activeTab">
                            <OverviewPage :active-tab="activeTab" :telemetry="telemetry.data" :connected="telemetry.connected" />
                            <TruckPage :active-tab="activeTab" :telemetry="telemetry.data" :connected="telemetry.connected" />
                            <LogbookPage :active-tab="activeTab" :telemetry="telemetry.data" :connected="telemetry.connected" />
                            <TransactionsPage :active-tab="activeTab" :telemetry="telemetry.data" :connected="telemetry.connected" />
                            <DebugPage :active-tab="activeTab" :telemetry="telemetry.data" :connected="telemetry.connected" />
                        </div>
                    </transition>
                </main>
            </div>
        </div>
    </div>
</template>