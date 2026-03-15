<script setup lang="ts">
    import { computed, onMounted, ref } from 'vue';
    import { useRouter } from 'vue-router';

    import { useTelemetryStore } from '@stores/telemetry';
    import { useLayoutStore } from '@stores/layout';
    import { usePluginBridgeStore } from '@stores/pluginBridge';
    import { useMiscStore } from '@stores/misc';
    import { useEventsStore } from '@stores/events';
    import { useSettingsStore } from '@stores/settings';
    import { Locale } from '@composables/useLanguage';
    import { i18n, setLocale } from '../i18n';
    import type { SupportedLocale } from '../i18n';

    import {
        faArrowLeft,
        faGaugeHigh,
        faRoute,
        faTruck,
        faSuitcase,
        faBug,
        faWrench,
        faLanguage,
        faPlugCircleBolt,
        faCircleExclamation,
        faTriangleExclamation,
        faWallet,
        faArrowTrendUp,
        faArrowTrendDown,
    } from '@fortawesome/free-solid-svg-icons';

    type TabId = 'overview' | 'truck' | 'job' | 'transactions' | 'debug';

    const router = useRouter();
    const telemetry = useTelemetryStore();
    const layout = useLayoutStore();
    const settings = useSettingsStore();
    const bridge = usePluginBridgeStore();
    const misc = useMiscStore();
    const eventsStore = useEventsStore();

    const activeTab = ref<TabId>('overview');
    const moneyDelta = ref(1000);
    const transactionFilter = ref<'all' | 'income' | 'expense'>('all');
    const activeLocale = computed(() => i18n.global.locale.value as SupportedLocale);

    const speedUnitLabel = computed(() => (settings.settings.speedUnit === 'mph' ? 'mph' : 'km/h'));
    const isMph = computed(() => settings.settings.speedUnit === 'mph');

    function normalizeLocale(value: string | null | undefined): SupportedLocale {
        const raw = (value ?? '').trim().toLowerCase();
        const base = raw.split(/[-_]/)[0] ?? '';
        return base === 'pl' ? 'pl' : 'en';
    }

    const detectedLocale = computed<SupportedLocale>(() => {
        const langs = (navigator.languages && navigator.languages.length > 0)
            ? navigator.languages
            : [navigator.language];

        for (const lang of langs) {
            const normalized = normalizeLocale(lang);
            if (normalized === 'pl' || normalized === 'en') return normalized;
        }
        return 'en';
    });

    const detectedLocaleLabel = computed(() =>
        detectedLocale.value === 'pl'
            ? Locale('settings.language.polish')
            : Locale('settings.language.english')
    );

    async function chooseLanguage(locale: SupportedLocale) {
        await setLocale(locale, { persist: true });
    }

    function toDisplaySpeed(kmh: number): number {
        const value = Number(kmh ?? 0);
        if (!Number.isFinite(value)) return 0;
        return isMph.value ? value * 0.621371 : value;
    }

    function formatDistance(meters: number): string {
        const value = Number(meters ?? 0);
        if (!Number.isFinite(value) || value <= 0) return '--';
        if (value >= 1000) return `${(value / 1000).toFixed(1)} km`;
        return `${Math.round(value)} m`;
    }

    function formatDuration(seconds: number): string {
        const value = Number(seconds ?? 0);
        if (!Number.isFinite(value) || value <= 0) return '--';
        const h = Math.floor(value / 3600);
        const m = Math.floor((value % 3600) / 60);
        if (h > 0) return `${h}h ${m.toString().padStart(2, '0')}m`;
        return `${m} min`;
    }

    const speedDisplay = computed(() => Math.round(toDisplaySpeed(telemetry.data.speed)));
    const speedLimitDisplay = computed(() => Math.round(toDisplaySpeed(telemetry.data.speedLimit)));
    const cruiseDisplay = computed(() => Math.round(toDisplaySpeed(telemetry.data.cruiseControlSpeed)));

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
        { id: 'job' as const, label: Locale('pauseMenu2.tabs.job'), icon: faSuitcase },
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

    function formatTime(timestamp?: number) {
        if (!timestamp) return '--:--';
        return new Date(timestamp).toLocaleTimeString(undefined, {
            hour: '2-digit', minute: '2-digit', second: '2-digit'
        });
    }

    const filteredTransactions = computed(() => {
        if (transactionFilter.value === 'income') {
            return eventsStore.transactionHistory.filter(t => t.income === 'true');
        }
        if (transactionFilter.value === 'expense') {
            return eventsStore.transactionHistory.filter(t => t.income !== 'true');
        }
        return eventsStore.transactionHistory;
    });

    const disableEconomyTools = computed(() => !bridge.available || misc.miscSettings.isMultiplayer);

    onMounted(() => refreshInfo());
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
                                <FontAwesomeIcon :icon="faTriangleExclamation" />
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
                    <section v-if="activeTab === 'overview'" key="overview" class="pm2__grid">
                        <div class="pm2__card hud-glass">
                            <div class="pm2__card-head">
                                <FontAwesomeIcon :icon="faGaugeHigh" />
                                <span>{{ Locale('pauseMenu2.overview.title') }}</span>
                            </div>
                            <div class="pm2__kv">
                                <div><span>{{ Locale('pauseMenu2.overview.speed') }}</span><strong>{{ speedDisplay }} {{ speedUnitLabel }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.overview.speedLimit') }}</span><strong>{{ speedLimitDisplay > 0 ? speedLimitDisplay : '--' }} {{ speedUnitLabel }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.overview.cruise') }}</span><strong>{{ telemetry.data.cruiseControl ? `${cruiseDisplay} ${speedUnitLabel}` : '--' }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.overview.navDistance') }}</span><strong>{{ formatDistance(telemetry.data.navDistance) }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.overview.navTime') }}</span><strong>{{ formatDuration(telemetry.data.navTime) }}</strong></div>
                            </div>
                        </div>

                        <div class="pm2__card hud-glass">
                            <div class="pm2__card-head">
                                <FontAwesomeIcon :icon="faCircleExclamation" />
                                <span>{{ Locale('pauseMenu2.warnings.title') }}</span>
                            </div>
                            <div class="pm2__warnings">
                                <div v-if="!telemetry.connected" class="pm2__warn pm2__warn--bad">
                                    <FontAwesomeIcon :icon="faTriangleExclamation" />
                                    <span>{{ Locale('pauseMenu2.warnings.disconnected') }}</span>
                                </div>
                                <div v-if="misc.miscSettings.gameMismatch" class="pm2__warn pm2__warn--warn">
                                    <FontAwesomeIcon :icon="faTriangleExclamation" />
                                    <span>{{ Locale('pauseMenu.warnings.gameMismatch') }}</span>
                                </div>
                                <div v-if="misc.miscSettings.frameworkMismatch" class="pm2__warn pm2__warn--warn">
                                    <FontAwesomeIcon :icon="faTriangleExclamation" />
                                    <span>{{ Locale('pauseMenu.warnings.frameworkMismatch') }}</span>
                                </div>
                                <div
                                    v-if="telemetry.connected && !misc.miscSettings.gameMismatch && !misc.miscSettings.frameworkMismatch"
                                    class="pm2__warn pm2__warn--ok"
                                >
                                    <span>{{ Locale('pauseMenu2.warnings.none') }}</span>
                                </div>
                            </div>
                        </div>

                        <div class="pm2__card hud-glass pm2__card--wide">
                            <div class="pm2__card-head">
                                <FontAwesomeIcon :icon="faWrench" />
                                <span>{{ Locale('pauseMenu2.actions.title') }}</span>
                            </div>
                            <div class="flex flex-wrap gap-2.5">
                                <button class="pm2__btn pm2__btn--ghost animate__animated animate__fadeInUp animate__faster" style="animation-delay: 35ms" @click="toggleEditMode">
                                    <FontAwesomeIcon :icon="faWrench" />
                                    <span>{{ layout.editMode ? Locale('pauseMenu2.finishEdit') : Locale('pauseMenu2.editLayout') }}</span>
                                </button>
                            </div>
                        </div>

                        <div class="pm2__card hud-glass pm2__card--wide">
                            <div class="pm2__lang">
                                <div class="pm2__lang-head">
                                    <div class="pm2__lang-title">
                                        <FontAwesomeIcon :icon="faLanguage" />
                                        <span>{{ Locale('settings.language.selector') }}</span>
                                    </div>
                                    <div class="pm2__lang-detected">
                                        {{ Locale('pauseMenu2.language.detected') }}: <strong>{{ detectedLocaleLabel }}</strong>
                                    </div>
                                </div>
                                <div class="flex flex-wrap gap-2.5">
                                    <button
                                        class="pm2__lang-btn"
                                        :class="activeLocale === 'en' ? 'pm2__lang-btn--active' : ''"
                                        @click="chooseLanguage('en')"
                                        :title="Locale('settings.language.english')"
                                    >
                                        <span class="pm2__flag">🇺🇸</span>
                                        <span class="pm2__lang-label">{{ Locale('settings.language.english') }}</span>
                                    </button>
                                    <button
                                        class="pm2__lang-btn"
                                        style="animation-delay: 35ms"
                                        :class="activeLocale === 'pl' ? 'pm2__lang-btn--active' : ''"
                                        @click="chooseLanguage('pl')"
                                        :title="Locale('settings.language.polish')"
                                    >
                                        <span class="pm2__flag">🇵🇱</span>
                                        <span class="pm2__lang-label">{{ Locale('settings.language.polish') }}</span>
                                    </button>
                                    <button
                                        class="pm2__lang-btn pm2__lang-btn--auto"
                                        style="animation-delay: 70ms"
                                        :class="activeLocale === detectedLocale ? 'pm2__lang-btn--active' : ''"
                                        @click="chooseLanguage(detectedLocale)"
                                        :title="Locale('pauseMenu2.language.autoTitle')"
                                    >
                                        <span class="pm2__flag">{{ detectedLocale === 'pl' ? '🇵🇱' : '🇺🇸' }}</span>
                                        <span class="pm2__lang-label">{{ Locale('pauseMenu2.language.auto') }}</span>
                                    </button>
                                </div>
                            </div>
                        </div>
                    </section>

                    <section v-else-if="activeTab === 'truck'" key="truck" class="pm2__grid">
                        <div class="pm2__card hud-glass pm2__card--wide">
                            <div class="pm2__card-head">
                                <FontAwesomeIcon :icon="faTruck" />
                                <span>{{ Locale('pauseMenu2.truck.title') }}</span>
                            </div>
                            <div class="pm2__kv pm2__kv--cols">
                                <div><span>{{ Locale('pauseMenu2.truck.model') }}</span><strong>{{ telemetry.data.truckMake }} {{ telemetry.data.truckModel }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.plate') }}</span><strong>{{ telemetry.data.truckLicensePlate }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.odometer') }}</span><strong>{{ Math.round(telemetry.data.odometer).toLocaleString() }} km</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.fuel') }}</span><strong>{{ Math.round(telemetry.data.fuel) }} / {{ Math.round(telemetry.data.fuelMax) }} L</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.adblue') }}</span><strong>{{ Math.round(telemetry.data.adblue) }} / {{ Math.round(telemetry.data.adblueMax) }} L</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.wear') }}</span><strong>{{ Math.round((1 - telemetry.data.truckWear) * 100) }}%</strong></div>
                            </div>
                        </div>

                        <div class="pm2__card hud-glass">
                            <div class="pm2__card-head">
                                <FontAwesomeIcon :icon="faWrench" />
                                <span>{{ Locale('pauseMenu2.truck.components') }}</span>
                            </div>
                            <div class="pm2__kv">
                                <div><span>{{ Locale('pauseMenu2.truck.engine') }}</span><strong>{{ Math.round((1 - telemetry.data.engineWear) * 100) }}%</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.transmission') }}</span><strong>{{ Math.round((1 - telemetry.data.transmissionWear) * 100) }}%</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.cabin') }}</span><strong>{{ Math.round((1 - telemetry.data.cabinWear) * 100) }}%</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.chassis') }}</span><strong>{{ Math.round((1 - telemetry.data.chassisWear) * 100) }}%</strong></div>
                                <div><span>{{ Locale('pauseMenu2.truck.wheels') }}</span><strong>{{ Math.round((1 - telemetry.data.wheelsWear) * 100) }}%</strong></div>
                                <div v-if="telemetry.data.trailerAttached"><span>{{ Locale('pauseMenu2.truck.trailer') }}</span><strong>{{ Math.round((1 - telemetry.data.trailerWear) * 100) }}%</strong></div>
                            </div>
                        </div>
                    </section>

                    <section v-else-if="activeTab === 'job'" key="job" class="pm2__grid">
                        <div class="pm2__card hud-glass pm2__card--wide">
                            <div class="pm2__card-head">
                                <FontAwesomeIcon :icon="faRoute" />
                                <span>{{ Locale('pauseMenu2.job.title') }}</span>
                            </div>
                            <div class="pm2__kv pm2__kv--cols">
                                <div><span>{{ Locale('pauseMenu2.job.cargo') }}</span><strong>{{ telemetry.data.jobCargoName || '--' }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.job.from') }}</span><strong>{{ telemetry.data.jobSourceCity || '--' }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.job.to') }}</span><strong>{{ telemetry.data.jobDestinationCity || '--' }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.job.remaining') }}</span><strong>{{ formatDuration(telemetry.data.jobRemainingTime) }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.job.distanceRemaining') }}</span><strong>{{ formatDistance(telemetry.data.jobDistanceRemaining) }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.job.income') }}</span><strong>{{ Math.round(telemetry.data.jobIncome).toLocaleString() }}</strong></div>
                            </div>
                        </div>
                    </section>

                    <section v-else-if="activeTab === 'transactions'" key="transactions" class="pm2__grid">
                        <div class="pm2__card hud-glass pm2__card--wide flex flex-col max-h-[500px]">
                            <div class="pm2__card-head justify-between w-full flex-none">
                                <div class="flex items-center gap-2">
                                    <FontAwesomeIcon :icon="faWallet" />
                                    <span>{{ Locale('pauseMenu2.transactions.title') }}</span>
                                </div>
                                
                                <div class="flex items-center gap-1">
                                    <button class="pm2__btn" :class="transactionFilter === 'all' ? 'pm2__btn--primary' : 'pm2__btn--ghost'" @click="transactionFilter = 'all'">{{ Locale('pauseMenu2.transactions.filterAll') }}</button>
                                    <button class="pm2__btn" :class="transactionFilter === 'income' ? 'border-emerald-500/50 text-emerald-400 bg-emerald-500/10' : 'pm2__btn--ghost'" @click="transactionFilter = 'income'">
                                        <FontAwesomeIcon :icon="faArrowTrendUp" />
                                    </button>
                                    <button class="pm2__btn" :class="transactionFilter === 'expense' ? 'border-rose-500/50 text-rose-400 bg-rose-500/10' : 'pm2__btn--ghost'" @click="transactionFilter = 'expense'">
                                        <FontAwesomeIcon :icon="faArrowTrendDown" />
                                    </button>
                                </div>
                            </div>

                            <div v-if="filteredTransactions.length === 0" class="flex-1 flex flex-col items-center justify-center py-6 text-sm opacity-50 font-mono">
                                {{ Locale('pauseMenu2.transactions.empty') }}
                            </div>
                            
                            <div v-else class="flex-1 flex flex-col gap-2 overflow-y-auto pr-2 custom-scrollbar">
                                <div v-for="t in filteredTransactions" :key="t.id" class="flex items-center justify-between p-3 rounded-xl border border-white/5 bg-black/20 hover:bg-white/5 transition-colors">
                                    <div class="flex items-center gap-3">
                                        <div class="w-8 h-8 rounded-full flex items-center justify-center shrink-0"
                                            :class="t.income === 'true' ? 'bg-emerald-500/20 text-emerald-400' : 'bg-rose-500/20 text-rose-400'">
                                            <FontAwesomeIcon :icon="t.income === 'true' ? faArrowTrendUp : faArrowTrendDown" />
                                        </div>
                                        <div class="flex flex-col justify-center">
                                            <span class="text-xs uppercase tracking-wider font-bold opacity-50 leading-none mb-1">{{ t.type === 'bank' ? Locale('widgets.alerts.bank') : Locale('widgets.alerts.xp') }}</span>
                                            <span class="text-sm font-semibold text-slate-100 leading-none">{{ Locale('widgets.alerts.' + t.description) }}</span>
                                        </div>
                                    </div>
                                    <div class="flex flex-col items-end">
                                        <span class="text-[10px] font-mono opacity-40 mb-1 leading-none">{{ formatTime(t.timestamp) }}</span>
                                        <span class="font-mono font-bold tracking-tight text-right leading-none" :class="t.income === 'true' ? 'text-emerald-400' : 'text-rose-400'">
                                            {{ t.income === 'true' ? '+' : '-' }}{{ (t.amount || 0).toLocaleString() }}$
                                        </span>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </section>

                    <section v-else key="debug" class="pm2__grid">
                        <div class="pm2__card hud-glass pm2__card--wide">
                            <div class="pm2__card-head">
                                <FontAwesomeIcon :icon="faBug" />
                                <span>{{ Locale('pauseMenu2.debug.title') }}</span>
                            </div>

                            <div class="pm2__kv pm2__kv--cols">
                                <div><span>{{ Locale('pauseMenu2.debug.bridge') }}</span><strong>{{ bridge.available ? Locale('pauseMenu2.debug.available') : Locale('pauseMenu2.debug.unavailable') }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.debug.webViewFocus') }}</span><strong>{{ misc.miscSettings.webViewFocus ? Locale('pauseMenu2.debug.on') : Locale('pauseMenu2.debug.off') }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.debug.editMode') }}</span><strong>{{ layout.editMode ? Locale('pauseMenu2.debug.on') : Locale('pauseMenu2.debug.off') }}</strong></div>
                                <div><span>{{ Locale('pauseMenu2.debug.multiplayer') }}</span><strong>{{ misc.miscSettings.isMultiplayer ? Locale('pauseMenu2.debug.on') : Locale('pauseMenu2.debug.off') }}</strong></div>
                            </div>

                            <div class="pm2__debug-tools">
                                <div class="pm2__debug-title">
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
                                <div class="pm2__hint" v-if="misc.miscSettings.isMultiplayer">{{ Locale('pauseMenu.multiplayerDisabled') }}</div>
                                <div class="pm2__hint pm2__hint--bad" v-if="bridge.lastError">{{ Locale('pauseMenu.error') }}: {{ bridge.lastError }}</div>
                            </div>
                        </div>
                    </section>
                    </transition>
                </main>
            </div>
        </div>
    </div>
</template>

<style scoped>
    .pm2 {
        position: fixed;
        inset: 0;
        color: var(--hud-text);
    }

    .pm2__bg {
        position: absolute;
        inset: 0;
        background:
            radial-gradient(1000px 600px at 20% 20%, rgba(94, 212, 255, 0.12), transparent 60%),
            radial-gradient(900px 500px at 80% 30%, rgba(94, 212, 255, 0.10), transparent 60%),
            linear-gradient(180deg, rgba(6, 10, 16, 0.92), rgba(4, 7, 12, 0.92));
    }

    .pm2__bg::before {
        content: '';
        position: absolute;
        inset: 0;
        background-image: url('../assets/pause-menu/scs_grid.svg');
        opacity: 0.38;
        pointer-events: none;
        mix-blend-mode: screen;
    }

    .pm2__shell {
        position: relative;
        height: 100%;
        display: grid;
        grid-template-rows: auto 1fr;
        padding: 16px;
        gap: 12px;
    }

    .pm2__header {
        display: flex;
        align-items: center;
        justify-content: space-between;
        border-radius: 16px;
        padding: 12px 14px;
        box-shadow:
            0 0 0 1px rgba(255,255,255,0.02),
            0 16px 36px rgba(0,0,0,0.55),
            inset 0 1px 0 rgba(255,255,255,0.05);
    }

    .pm2__brand {
        display: flex;
        align-items: center;
        gap: 12px;
        min-width: 0;
    }

    .pm2__logo {
        width: 52px;
        height: 52px;
        border-radius: 14px;
        display: grid;
        place-items: center;
        border: 1px solid rgba(94, 212, 255, 0.78);
        background:
            linear-gradient(180deg, rgba(94, 212, 255, 0.18), rgba(10, 14, 22, 0.92)),
            url('../assets/pause-menu/scs_stripes.svg');
        box-shadow: inset 0 1px 0 rgba(255,255,255,0.06);
        flex: 0 0 auto;
    }

    .pm2__logo span {
        letter-spacing: 0.08em;
        font-weight: 800;
        color: #67e8f9;
        text-shadow: 0 0 10px rgba(94, 212, 255, 0.25);
        font-size: 14px;
    }

    .pm2__title {
        display: grid;
        gap: 6px;
        min-width: 0;
    }

    .pm2__title-top {
        display: flex;
        flex-wrap: wrap;
        align-items: center;
        gap: 8px;
        min-width: 0;
    }

    .pm2__paused {
        font-size: 12px;
        letter-spacing: 0.16em;
        text-transform: uppercase;
        color: #67e8f9;
    }

    .pm2__badge {
        display: inline-flex;
        align-items: center;
        gap: 8px;
        padding: 6px 10px;
        border-radius: 9999px;
        border: 1px solid rgba(170, 200, 230, 0.22);
        background: rgba(10, 14, 22, 0.58);
        font-size: 12px;
        color: rgba(203, 213, 225, 0.95);
    }

    .pm2__badge--ok {
        border-color: rgba(108, 224, 160, 0.35);
        color: rgba(108, 224, 160, 0.92);
    }

    .pm2__badge--bad {
        border-color: rgba(255, 79, 73, 0.35);
        color: rgba(255, 170, 170, 0.95);
    }

    .pm2__badge--bad {
        animation-duration: 2.2s;
    }

    .pm2__title-bottom {
        display: flex;
        flex-wrap: wrap;
        align-items: center;
        gap: 8px;
        font-size: 13px;
        min-width: 0;
    }

    .pm2__sep {
        opacity: 0.45;
    }

    .pm2__header-actions {
        display: flex;
        align-items: center;
        gap: 10px;
        flex: 0 0 auto;
    }

    .pm2__btn {
        display: inline-flex;
        align-items: center;
        gap: 8px;
        border: 1px solid rgba(148, 163, 184, 0.32);
        background: rgba(6, 10, 16, 0.74);
        color: #dce6f4;
        border-radius: 12px;
        padding: 9px 12px;
        cursor: pointer;
        font-family: inherit;
        font-size: 12px;
        box-shadow: inset 0 1px 0 rgba(255,255,255,0.03);
        transition: border-color 120ms ease, transform 120ms ease, background 120ms ease;
    }

    .pm2__btn:hover {
        border-color: rgba(94, 212, 255, 0.78);
        color: #67e8f9;
        transform: translateY(-1px);
    }

    .pm2__btn:disabled {
        opacity: 0.55;
        cursor: not-allowed;
        transform: none;
    }

    .pm2__btn--primary {
        background: rgba(6, 10, 16, 0.74);
        border-color: rgba(94, 212, 255, 0.78);
        color: #67e8f9;
    }

    .pm2__btn--ghost {
        background: rgba(10, 14, 22, 0.45);
    }

    .pm2__body {
        min-height: 0;
        display: grid;
        grid-template-columns: 250px 1fr;
        gap: 12px;
    }

    .pm2__nav {
        border-radius: 16px;
        padding: 10px;
        min-height: 0;
        display: grid;
        align-content: start;
        gap: 8px;
        box-shadow:
            0 0 0 1px rgba(255,255,255,0.02),
            0 14px 32px rgba(0,0,0,0.52),
            inset 0 1px 0 rgba(255,255,255,0.05);
    }

    .pm2__nav-item {
        display: grid;
        grid-template-columns: 18px 1fr;
        align-items: center;
        gap: 10px;
        border-radius: 12px;
        padding: 10px 12px;
        background: rgba(10, 14, 22, 0.35);
        border: 1px solid rgba(148, 163, 184, 0.18);
        cursor: pointer;
        color: rgba(203, 213, 225, 0.92);
        font-family: inherit;
        font-size: 12px;
        text-align: left;
        transition: border-color 120ms ease, background 120ms ease, transform 120ms ease;
    }

    .pm2__nav-item:hover {
        border-color: rgba(94, 212, 255, 0.32);
        background: rgba(10, 14, 22, 0.46);
        transform: translateY(-1px);
    }

    .pm2__nav-item--active {
        border-color: rgba(94, 212, 255, 0.78);
        background: rgba(6, 10, 16, 0.74);
        color: #67e8f9;
    }

    .pm2__nav-ico {
        opacity: 0.95;
    }

    .pm2__nav-divider {
        height: 1px;
        background: rgba(148, 163, 184, 0.18);
        margin: 6px 4px;
    }

    .pm2__content {
        min-height: 0;
        overflow: auto;
        padding: 2px;
    }

    .pm2__grid {
        display: grid;
        grid-template-columns: repeat(12, minmax(0, 1fr));
        gap: 12px;
        padding-bottom: 12px;
    }

    .pm2__card {
        grid-column: span 12;
        border-radius: 16px;
        padding: 14px;
        box-shadow:
            0 0 0 1px rgba(255,255,255,0.02),
            0 14px 32px rgba(0,0,0,0.52),
            inset 0 1px 0 rgba(255,255,255,0.05);
    }

    .pm2__card--wide {
        grid-column: span 12;
    }

    .pm2__card-head {
        display: inline-flex;
        align-items: center;
        gap: 10px;
        font-size: 12px;
        text-transform: uppercase;
        letter-spacing: 0.12em;
        color: rgba(203, 213, 225, 0.92);
        margin-bottom: 12px;
    }

    .pm2__kv {
        display: grid;
        gap: 10px;
    }

    .pm2__kv--cols {
        grid-template-columns: repeat(2, minmax(0, 1fr));
        gap: 10px 14px;
    }

    .pm2__kv div {
        display: flex;
        justify-content: space-between;
        gap: 10px;
        border-bottom: 1px solid rgba(71, 85, 105, 0.42);
        padding-bottom: 6px;
        font-size: 13px;
        color: rgba(143, 157, 177, 0.95);
    }

    .pm2__kv strong {
        color: rgba(230, 237, 247, 0.96);
        font-weight: 800;
        letter-spacing: 0.01em;
    }

    .pm2__warnings {
        display: grid;
        gap: 10px;
    }

    .pm2__warn {
        display: flex;
        align-items: center;
        gap: 10px;
        padding: 10px 12px;
        border-radius: 14px;
        border: 1px solid rgba(148, 163, 184, 0.22);
        background: rgba(10, 14, 22, 0.45);
        color: rgba(203, 213, 225, 0.92);
        font-size: 13px;
    }

    .pm2__warn--ok {
        border-color: rgba(108, 224, 160, 0.28);
        color: rgba(108, 224, 160, 0.9);
    }

    .pm2__warn--warn {
        border-color: rgba(94, 212, 255, 0.35);
        color: rgba(103, 232, 249, 0.95);
        background: rgba(6, 10, 16, 0.52);
    }

    .pm2__warn--bad {
        border-color: rgba(255, 79, 73, 0.32);
        color: rgba(255, 170, 170, 0.95);
        background: rgba(255, 79, 73, 0.06);
    }

    .pm2__lang {
        display: grid;
        gap: 10px;
    }

    .pm2__lang-head {
        display: flex;
        align-items: baseline;
        justify-content: space-between;
        gap: 10px;
        flex-wrap: wrap;
    }

    .pm2__lang-title {
        display: inline-flex;
        align-items: center;
        gap: 10px;
        font-size: 12px;
        letter-spacing: 0.14em;
        text-transform: uppercase;
        color: rgba(203, 213, 225, 0.82);
    }

    .pm2__lang-detected {
        font-size: 12px;
        color: rgba(143, 157, 177, 0.95);
    }

    .pm2__lang-detected strong {
        color: rgba(230, 237, 247, 0.96);
        font-weight: 800;
    }

    .pm2__lang-btn {
        display: inline-flex;
        align-items: center;
        gap: 10px;
        border-radius: 12px;
        padding: 8px 10px;
        background: rgba(10, 14, 22, 0.38);
        border: 1px solid rgba(148, 163, 184, 0.22);
        cursor: pointer;
        color: rgba(203, 213, 225, 0.92);
        font-family: inherit;
        font-size: 12px;
        transition: border-color 120ms ease, transform 120ms ease, background 120ms ease;
    }

    .pm2__lang-btn:hover {
        border-color: rgba(94, 212, 255, 0.32);
        background: rgba(10, 14, 22, 0.46);
        transform: translateY(-1px);
    }

    .pm2__lang-btn--active {
        border-color: rgba(94, 212, 255, 0.78);
        background: rgba(6, 10, 16, 0.74);
        color: #67e8f9;
    }

    .pm2__lang-btn--auto {
        border-style: dashed;
    }

    .pm2__flag {
        font-size: 16px;
        line-height: 1;
        filter: drop-shadow(0 0 10px rgba(0, 0, 0, 0.35));
    }

    .pm2__lang-label {
        white-space: nowrap;
    }

    .pm2__debug-tools {
        margin-top: 14px;
        border-top: 1px solid rgba(148, 163, 184, 0.18);
        padding-top: 14px;
        display: grid;
        gap: 10px;
    }

    .pm2__debug-title {
        display: inline-flex;
        align-items: center;
        gap: 10px;
        font-size: 12px;
        letter-spacing: 0.14em;
        text-transform: uppercase;
        color: rgba(203, 213, 225, 0.82);
    }

    .pm2__debug-row {
        display: flex;
        flex-wrap: wrap;
        gap: 10px;
        align-items: center;
    }

    .pm2__input {
        width: 160px;
        border: 1px solid rgba(148, 163, 184, 0.32);
        background: rgba(6, 10, 16, 0.74);
        color: rgba(230, 237, 247, 0.96);
        border-radius: 12px;
        padding: 10px 12px;
        font-family: inherit;
        font-size: 12px;
    }

    .pm2__hint {
        color: rgba(143, 157, 177, 0.95);
        font-size: 12px;
    }

    .pm2__hint--bad {
        color: rgba(255, 170, 170, 0.95);
    }

    @media (min-width: 900px) {
        .pm2__card:nth-child(1) { grid-column: span 6; }
        .pm2__card:nth-child(2) { grid-column: span 6; }
        .pm2__card:nth-child(3) { grid-column: span 12; }
    }

    @media (max-width: 720px) {
        .pm2__shell {
            padding: 10px;
        }
        .pm2__body {
            grid-template-columns: 1fr;
        }
        .pm2__nav {
            grid-auto-flow: column;
            grid-auto-columns: 1fr;
            overflow: auto;
        }
        .pm2__nav-divider {
            display: none;
        }
    }
</style>
