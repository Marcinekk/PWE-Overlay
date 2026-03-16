<script setup lang="ts">
    import { Locale } from '@composables/useLanguage';
    import { faRoute } from '@fortawesome/free-solid-svg-icons';

    import type { TelemetryData } from '@interfaces/telemetry';

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

    const props = defineProps<{
        activeTab: string;
        telemetry: TelemetryData;
        connected: boolean;
    }>();
</script>

<template>
    <section v-show="activeTab === 'job'" key="job" class="pm2__grid">
        <div class="pm2__card hud-glass pm2__card--wide">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faRoute" />
                <span>{{ Locale('pauseMenu2.job.title') }}</span>
            </div>
            <div class="pm2__kv pm2__kv--cols">
                <div><span>{{ Locale('pauseMenu2.job.cargo') }}</span><strong>{{ telemetry.jobCargoName || '--' }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.job.from') }}</span><strong>{{ telemetry.jobSourceCity || '--' }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.job.to') }}</span><strong>{{ telemetry.jobDestinationCity || '--' }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.job.remaining') }}</span><strong>{{ formatDuration(telemetry.jobRemainingTime) }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.job.distanceRemaining') }}</span><strong>{{ formatDistance(telemetry.jobDistanceRemaining) }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.job.income') }}</span><strong>{{ Math.round(telemetry.jobIncome).toLocaleString() }}</strong></div>
            </div>
        </div>
    </section>
</template>