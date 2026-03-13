<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { useSettingsStore } from '@stores/settings';
    import { Locale } from '@composables/useLanguage';

    const telemetry = useTelemetryStore();
    const settings = useSettingsStore();

    const speedLimitDisplay = computed(() => {
        const lim = telemetry.data.navSpeedLimit || telemetry.data.speedLimit;
        return Math.round(settings.settings.speedUnit === 'kmh' ? lim : lim * 0.621371);
    });

    const speedLimitStyle = computed(() => {
        return telemetry.isSpeeding
            ? 'bg-red-600/25 text-red-300 ring-2 ring-red-500'
            : 'bg-white text-slate-900 ring-2 ring-red-500';
    });
</script>

<template>
    <div class="widget-panel w-full px-4 py-3">
        <div class="flex items-center justify-between">
            <div class="text-center">
                <div class="text-base font-black font-mono text-slate-100 leading-none">{{ telemetry.navDistanceFormatted }}</div>
                <div class="text-xs text-slate-400 font-mono">{{ Locale('widgets.navigation.to_destination') }}</div>
            </div>

            <div class="h-10 w-px bg-slate-600/60" />

            <div class="text-center">
                <div class="text-base font-black font-mono text-slate-100 leading-none">{{ telemetry.navTimeFormatted }}</div>
                <div class="text-xs text-slate-400 font-mono">{{ Locale('widgets.navigation.eta') }}</div>
            </div>

            <div class="h-10 w-px bg-slate-600/60" />

            <div class="text-center">
                <div class="relative">
                    <div
                        class="w-5 h-5 rounded-full flex items-center justify-center font-black font-mono text-sm"
                        :class="speedLimitStyle"
                    >
                        {{ speedLimitDisplay }}
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>
