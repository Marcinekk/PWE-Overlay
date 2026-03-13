<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { Locale } from '@composables/useLanguage';

    const telemetry = useTelemetryStore();

    const shouldShiftUp = computed(() => {
        const moving = telemetry.data.speed > 3;
        const forwardGear = telemetry.data.gear > 0;
        const highRpm = telemetry.rpmPercent >= 82;
        return telemetry.data.engineOn && moving && forwardGear && highRpm;
    });

    const gearColor = computed(() => {
        if (shouldShiftUp.value) return 'gear-shift-warning';
        const g = telemetry.data.gear;
        if (g === 0) return 'text-slate-500';
        if (g < 0) return 'text-red-400';
        return 'text-cyan-300';
    });
</script>

<template>
    <div class="widget-panel w-full h-full px-3 py-3 flex flex-col items-center justify-center gap-1">
        <div
            class="text-4xl font-black font-mono leading-none transition-all duration-75"
            :class="gearColor"
        >
            {{ telemetry.gearLabel }}
        </div>

        <div
            class="text-xs text-slate-500 font-mono uppercase tracking-widest"
            :class="telemetry.data.cruiseControlSpeed > 0 ? '' : 'opacity-0'"
        >
            {{ Math.round((telemetry.data.cruiseControlSpeed || 0)) }}
        </div>

        <div class="flex gap-1 mt-0.5">
            <div
                class="text-xs font-bold px-1.5 py-0.5 rounded transition-all"
                :class="telemetry.data.parkBrake ? 'bg-red-500/20 text-red-300 ring-1 ring-red-500' : 'bg-slate-800 text-slate-500'"
            >
                {{ Locale('widgets.gear.park') }}
            </div>

            <div
                class="text-xs font-bold px-1.5 py-0.5 rounded transition-all"
                :class="telemetry.data.cruiseControl ? 'bg-yellow-500/20 text-yellow-300 ring-1 ring-yellow-500' : 'bg-slate-800 text-slate-500'"
            >
                {{ Locale('widgets.gear.cruise') }}
            </div>

            <div
                class="text-xs font-bold px-1.5 py-0.5 rounded transition-all"
                :class="telemetry.data.motorBrake ? 'bg-yellow-500/20 text-yellow-300 ring-1 ring-yellow-500' : 'bg-slate-800 text-slate-500'"
            >
                {{ Locale('widgets.gear.motor_brake') }}
            </div>
        </div>
    </div>
</template>

<style scoped>
    .gear-shift-warning {
        color: #ff8b86;
        animation: gearShiftBlink 0.9s ease-in-out infinite;
        text-shadow: 0 0 8px rgba(255, 79, 73, 0.55);
    }

    @keyframes gearShiftBlink {
        0%, 100% {
            color: #ff8b86;
            text-shadow: 0 0 6px rgba(255, 79, 73, 0.45);
        }
        50% {
            color: #ff4f49;
            text-shadow: 0 0 12px rgba(255, 79, 73, 0.75);
        }
    }
</style>
