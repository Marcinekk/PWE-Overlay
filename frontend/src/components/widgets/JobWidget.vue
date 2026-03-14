<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { Locale } from '@composables/useLanguage';

    import { faBriefcase, faClock, faArrowRight } from '@fortawesome/free-solid-svg-icons';

    const telemetry = useTelemetryStore();

    const progressPct = computed(() => {
        const total = telemetry.data.jobDistance;
        const rem = telemetry.data.jobDistanceRemaining / 1000;
        if (!total) return 0;
        return Math.max(0, Math.min(100, ((total - rem) / total) * 100));
    });

    const cargoMassTons = computed(() => (telemetry.data.trailerCargoMass / 1000).toFixed(1));
    const timeWarning = computed(() => telemetry.data.jobRemainingTime > 0 && telemetry.data.jobRemainingTime < 3600);
    const incomeFormatted = computed(() => {
        const n = telemetry.data.jobIncome;
        if (!n) return '---';
        return telemetry.data.gameName === 'ats' ? n.toLocaleString('pl-PL') + ' $' : n.toLocaleString('pl-PL') + ' €';
    });
</script>

<template>
    <transition
        enter-active-class="animate__animated animate__zoomIn animate__faster"
        leave-active-class="animate__animated animate__zoomOut animate__faster"
    >
        <div class="widget-panel w-full h-auto px-4 py-3 overflow-auto">
            <div class="flex items-center gap-2 mb-1">
                <FontAwesomeIcon :icon="faBriefcase" class="text-cyan-300" />
                <span class="text-xs text-slate-400 uppercase tracking-wider font-mono">{{ Locale('widgets.job.name') }}</span>

                <div class="ml-auto flex items-center gap-1.5">
                    <FontAwesomeIcon :icon="faClock" class="text-slate-500 text-xs" />
                    <span class="text-xs font-mono" :class="timeWarning ? 'text-red-400 animate-pulse' : 'text-slate-300'">
                        {{ telemetry.jobRemainingFormatted }}
                    </span>
                </div>
            </div>

            <div class="flex items-center gap-2 mb-1">
                <div class="flex-1 text-center">
                    <div class="text-sm font-bold text-slate-100">{{ telemetry.data.jobSourceCity || '---' }}</div>
                    <div class="text-xs text-slate-500 truncate">{{ telemetry.data.jobSourceCompany }}</div>
                </div>

                <div class="flex flex-col items-center gap-0.5 text-slate-500">
                    <div class="w-12 h-px bg-slate-700 relative">
                        <div
                            class="absolute top-0 left-0 h-full bg-cyan-400 transition-all"
                            :style="{ width: `${progressPct}%` }"
                        />
                    </div>
                    <FontAwesomeIcon :icon="faArrowRight" class="text-xs" />
                </div>

                <div class="flex-1 text-center">
                    <div class="text-sm font-bold text-slate-100">{{ telemetry.data.jobDestinationCity || '---' }}</div>
                    <div class="text-xs text-slate-500 truncate">{{ telemetry.data.jobDestinationCompany }}</div>
                </div>
            </div>

            <div class="grid grid-cols-3 gap-1.5">
                <div class="bg-slate-800/45 rounded p-1.5 text-center col-span-2">
                    <div class="text-xs text-slate-500 font-mono">{{ Locale('widgets.job.cargo') }}</div>
                    <div class="text-xs font-bold text-cyan-300 truncate">{{ telemetry.data.jobCargoName || '---' }}</div>
                    <div class="text-xs text-slate-500">{{ cargoMassTons }} t</div>
                </div>

                <div class="bg-slate-800/45 rounded p-1.5 text-center">
                    <div class="text-xs text-slate-500 font-mono">{{ Locale('widgets.job.distance') }}</div>
                    <div class="text-xs font-bold text-slate-100">{{ Math.round(telemetry.data.jobDistanceRemaining / 1000) || telemetry.data.jobDistance }}</div>
                    <div class="text-xs text-slate-500">km</div>
                </div>
            </div>

            <div class="mt-0.5 text-center text-xs text-slate-500 font-mono">
                {{ Locale('widgets.job.income') }}: <span class="text-green-400 font-bold">{{ incomeFormatted }}</span>
            </div>
        </div>
    </transition>
</template>
