<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { useSettingsStore } from '@stores/settings';
    import { Locale } from '@composables/useLanguage';
    import { faTruck } from '@fortawesome/free-solid-svg-icons';

    const telemetry = useTelemetryStore();
    const settings = useSettingsStore();
    const odometer = computed(() => {
        const val = telemetry.data.odometer;
        return settings?.settings?.speedUnit === 'kmh' ? (val * 3.6).toLocaleString('pl-PL') + ' km' : (val * 2.23694).toLocaleString('pl-PL') + ' mi';
    });
</script>

<template>
    <div class="widget-panel w-full px-4 py-3">
        <div class="flex items-center gap-2 mb-0.5">
            <FontAwesomeIcon :icon="faTruck" class="text-cyan-300" />
            <span class="text-xs text-slate-400 uppercase tracking-wider font-mono">{{ Locale('widgets.truck-info.name') }}</span>
        </div>
        <div class="flex items-center justify-between">
            <div>
                <div class="text-base font-bold text-slate-100 font-mono">
                    {{ telemetry.data.truckMake }} {{ telemetry.data.truckModel }}
                </div>
                <div class="text-xs text-slate-500 font-mono">{{ telemetry.data.truckLicensePlate }}</div>
            </div>
            <div class="text-right">
                <div class="text-xs text-slate-500 font-mono">{{ Locale('widgets.truck-info.odometer') }}</div>
                <div class="text-sm font-bold text-slate-100 font-mono">{{ odometer }}</div>
            </div>
        </div>
    </div>
</template>
