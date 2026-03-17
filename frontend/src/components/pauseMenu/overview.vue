<script setup lang="ts">
    import { computed } from 'vue';
    import { useSettingsStore } from '@stores/settings';
    import { useMiscStore } from '@stores/misc';
    import { useRouter } from 'vue-router';
    import { useLayoutStore } from '@stores/layout';

    import { i18n, setLocale } from '../../i18n';
    import { Locale } from '@composables/useLanguage';
    import { faGaugeHigh, faCircleExclamation, faWrench, faLanguage, faTriangleExclamation, faRoute } from '@fortawesome/free-solid-svg-icons';

    import type { SupportedLocale } from '../../i18n';
    import type { TelemetryData } from '@interfaces/telemetry';

    const props = defineProps<{
        activeTab: string;
        telemetry: TelemetryData;
        connected: boolean;
    }>();

    const settings = useSettingsStore();
    const misc = useMiscStore();
    const router = useRouter();
    const layout = useLayoutStore();

    function toDisplaySpeed(kmh: number): number {
        const value = Number(kmh ?? 0);
        if (!Number.isFinite(value)) return 0;
        return isMph.value ? value * 0.621371 : value;
    }

    const activeLocale = computed(() => i18n.global.locale.value as SupportedLocale);
    const speedUnitLabel = computed(() => (settings.settings.speedUnit === 'mph' ? 'mph' : 'km/h'));
    const speedDisplay = computed(() => Math.round(toDisplaySpeed(props?.telemetry?.speed)));
    const speedLimitDisplay = computed(() => Math.round(toDisplaySpeed(props?.telemetry?.speedLimit)));
    const cruiseDisplay = computed(() => Math.round(toDisplaySpeed(props?.telemetry?.cruiseControlSpeed)));
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

    function formatDuration(seconds: number): string {
        const value = Number(seconds ?? 0);
        if (!Number.isFinite(value) || value <= 0) return '--';
        const h = Math.floor(value / 3600);
        const m = Math.floor((value % 3600) / 60);
        if (h > 0) return `${h}h ${m.toString().padStart(2, '0')}m`;
        return `${m} min`;
    }

    function formatDistance(meters: number): string {
        const value = Number(meters ?? 0);
        if (!Number.isFinite(value) || value <= 0) return '--';
        if (value >= 1000) return `${(value / 1000).toFixed(1)} km`;
        return `${Math.round(value)} m`;
    }

    function toggleEditMode() {
        router.push('/');
        layout.toggleSettingsMenu();
    }
</script>

<template>
    <section v-show="activeTab === 'overview'" key="overview" class="pm2__grid">
        <div class="pm2__card hud-glass">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faGaugeHigh" />
                <span>{{ Locale('pauseMenu2.overview.title') }}</span>
            </div>
            <div class="pm2__kv">
                <div><span>{{ Locale('pauseMenu2.overview.speed') }}</span><strong>{{ speedDisplay }} {{ speedUnitLabel }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.speedLimit') }}</span><strong>{{ speedLimitDisplay > 0 ? speedLimitDisplay : '--' }} {{ speedUnitLabel }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.cruise') }}</span><strong>{{ telemetry.cruiseControl ? `${cruiseDisplay} ${speedUnitLabel}` : '--' }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.navDistance') }}</span><strong>{{ formatDistance(telemetry.navDistance) }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.navTime') }}</span><strong>{{ formatDuration(telemetry.navTime) }}</strong></div>
            </div>
        </div>

        <div class="pm2__card hud-glass">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faCircleExclamation" />
                <span>{{ Locale('pauseMenu2.warnings.title') }}</span>
            </div>
            <div class="pm2__warnings">
                <div v-if="!connected" class="pm2__warn pm2__warn--bad">
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
                    v-if="connected && !misc.miscSettings.gameMismatch && !misc.miscSettings.frameworkMismatch"
                    class="pm2__warn pm2__warn--ok"
                >
                    <span>{{ Locale('pauseMenu2.warnings.none') }}</span>
                </div>
            </div>
        </div>

        <div class="pm2__card hud-glass">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faRoute" />
                <span>{{ Locale('pauseMenu2.overview.job') }}</span>
            </div>
            <div class="pm2__kv pm2__kv--cols">
                <div><span>{{ Locale('pauseMenu2.overview.cargo') }}</span><strong>{{ telemetry.jobCargoName || '--' }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.from') }}</span><strong>{{ telemetry.jobSourceCity || '--' }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.to') }}</span><strong>{{ telemetry.jobDestinationCity || '--' }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.remaining') }}</span><strong>{{ formatDuration(telemetry.jobRemainingTime) }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.distanceRemaining') }}</span><strong>{{ formatDistance(telemetry.jobDistanceRemaining) }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.overview.income') }}</span><strong>{{ Math.round(telemetry.jobIncome).toLocaleString() }}</strong></div>
            </div>
        </div>

        <div class="pm2__card hud-glass">
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

            <div class="pm2__nav-divider" />

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
</template>