<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { faGasPump } from '@fortawesome/free-solid-svg-icons';
    import { Locale } from '@composables/useLanguage';

    const telemetry = useTelemetryStore();
    const fuelPct = computed(() => telemetry.fuelPercent);
    const fuelWarning = computed(() => telemetry.data.fuelWarning || fuelPct.value < 15);
    const fuelBarColor = computed(() => {
        if (fuelPct.value < 15) return 'bg-red-500';
        if (fuelPct.value < 30) return 'bg-yellow-400';
        return 'bg-cyan-400';
    });
</script>

<template>
    <div class="widget-panel w-full px-4 py-3 overflow-auto">
        <div class="flex items-center justify-between mb-2">
            <div class="flex items-center gap-2">
                <FontAwesomeIcon :icon="faGasPump" class="text-cyan-300 text-sm" />
                <span class="text-xs text-slate-400 uppercase tracking-wider font-mono">{{ Locale('widgets.fuel.name') }}</span>
            </div>
            <span
                class="text-xs font-bold font-mono"
                :class="fuelWarning ? 'text-red-400 animate-pulse' : 'text-slate-300'"
            >
                {{ Math.round(telemetry.data.fuel) }}L
            </span>
        </div>

        <div class="relative h-2 bg-slate-800 rounded-full overflow-hidden mb-1.5">
            <div
                class="h-full rounded-full transition-all duration-300"
                :class="fuelBarColor"
                :style="{ width: `${fuelPct}%` }"
            />

            <div v-for="n in 3" :key="n" class="absolute top-0 h-full w-px bg-slate-600" :style="{ left: `${n * 25}%` }" />
        </div>

        <div class="flex justify-between text-xs text-slate-500 font-mono mb-1">
            <span>0</span>
            <span>{{ Math.round(telemetry.data.fuelMax) }}L</span>
        </div>

        <div class="grid grid-cols-2 gap-2">
            <div class="bg-slate-800/45 rounded p-1.5 text-center">
                <div class="text-xs text-slate-500 font-mono">{{ Locale('widgets.fuel.range') }}</div>
                <div class="text-sm font-bold font-mono text-slate-100">{{ Math.round(telemetry.data.fuelRange) }} km</div>
            </div>
            <div class="bg-slate-800/45 rounded p-1.5 text-center">
                <div class="text-xs text-slate-500 font-mono">{{ Locale('widgets.fuel.consumption') }}</div>
                <div class="text-sm font-bold font-mono text-slate-100">
                    {{ telemetry.data.fuelAvgConsumption.toFixed(1) }}
                </div>
            </div>
        </div>
    </div>
</template>
