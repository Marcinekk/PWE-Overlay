<script setup lang="ts">
    import { computed, onMounted, onUnmounted, ref } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { Locale } from '@composables/useLanguage';

    type TachographMode = 'auto' | 'driving' | 'work' | 'break' | 'rest';

    interface TachographState {
        drivingSec: number;
        workSec: number;
        breakSec: number;
        restSec: number;
        continuousDrivingSec: number;
    }

    const STORAGE_KEY = 'spf.tachograph.state.v1';
    const telemetry = useTelemetryStore();

    const manualMode = ref<TachographMode>('auto');
    const lastTickMs = ref(0);
    const breakStreakSec = ref(0);
    const restStreakSec = ref(0);
    const state = ref<TachographState>({
        drivingSec: 0,
        workSec: 0,
        breakSec: 0,
        restSec: 0,
        continuousDrivingSec: 0,
    });

    let timer: ReturnType<typeof setInterval> | null = null;

    const activeMode = computed<TachographMode>(() => {
        if (manualMode.value !== 'auto') return manualMode.value;
        if (telemetry.data.speed > 2) return 'driving';
        if (!telemetry.data.engineOn) return 'rest';
        if (telemetry.data.parkBrake) return 'break';
        return 'work';
    });

    const dailyDrivingLimitSec = 9 * 3600;
    const continuousDrivingLimitSec = 4.5 * 3600;
    const dailyWorkLimitSec = 13 * 3600;

    const drivingPercent = computed(() => Math.min((state.value.drivingSec / dailyDrivingLimitSec) * 100, 100));
    const continuousPercent = computed(() => Math.min((state.value.continuousDrivingSec / continuousDrivingLimitSec) * 100, 100));
    const workPercent = computed(() => Math.min((state.value.workSec / dailyWorkLimitSec) * 100, 100));
    const needsBreak = computed(() => state.value.continuousDrivingSec >= continuousDrivingLimitSec);
    const drivingLimitReached = computed(() => state.value.drivingSec >= dailyDrivingLimitSec);

    function formatDuration(seconds: number) {
        const safe = Math.max(0, Math.floor(seconds));
        const h = Math.floor(safe / 3600).toString().padStart(2, '0');
        const m = Math.floor((safe % 3600) / 60).toString().padStart(2, '0');
        return `${h}:${m}`;
    }

    const drivingLabel = computed(() => formatDuration(state.value.drivingSec));
    const workLabel = computed(() => formatDuration(state.value.workSec));
    const breakLabel = computed(() => formatDuration(state.value.breakSec));
    const restLabel = computed(() => formatDuration(state.value.restSec));
    const continuousLabel = computed(() => formatDuration(state.value.continuousDrivingSec));

    function persistState() {
        const payload = {
            manualMode: manualMode.value,
            breakStreakSec: breakStreakSec.value,
            restStreakSec: restStreakSec.value,
            state: state.value,
            savedAt: Date.now(),
        };
        localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
    }

    function hydrateState() {
        const raw = localStorage.getItem(STORAGE_KEY);
        if (!raw) return;

        try {
            const parsed = JSON.parse(raw) as {
                manualMode?: TachographMode;
                breakStreakSec?: number;
                restStreakSec?: number;
                state?: Partial<TachographState>;
            };

            if (parsed.manualMode) manualMode.value = parsed.manualMode;
            if (Number.isFinite(parsed.breakStreakSec)) breakStreakSec.value = Number(parsed.breakStreakSec);
            if (Number.isFinite(parsed.restStreakSec)) restStreakSec.value = Number(parsed.restStreakSec);
            if (parsed.state) {
                state.value = {
                    drivingSec: Number(parsed.state.drivingSec ?? 0),
                    workSec: Number(parsed.state.workSec ?? 0),
                    breakSec: Number(parsed.state.breakSec ?? 0),
                    restSec: Number(parsed.state.restSec ?? 0),
                    continuousDrivingSec: Number(parsed.state.continuousDrivingSec ?? 0),
                };
            }
        } catch {
            localStorage.removeItem(STORAGE_KEY);
        }
    }

    function resetDailyCounters() {
        state.value.drivingSec = 0;
        state.value.workSec = 0;
        state.value.breakSec = 0;
        state.value.continuousDrivingSec = 0;
        breakStreakSec.value = 0;
    }

    function applyTick(deltaSec: number) {
        if (deltaSec <= 0 || deltaSec > 5) return;

        switch (activeMode.value) {
            case 'driving':
                state.value.drivingSec += deltaSec;
                state.value.continuousDrivingSec += deltaSec;
                breakStreakSec.value = 0;
                restStreakSec.value = 0;
                break;
            case 'work':
                state.value.workSec += deltaSec;
                breakStreakSec.value = 0;
                restStreakSec.value = 0;
                break;
            case 'break':
                state.value.breakSec += deltaSec;
                breakStreakSec.value += deltaSec;
                restStreakSec.value = 0;
                if (breakStreakSec.value >= 45 * 60) {
                    state.value.continuousDrivingSec = 0;
                }
                break;
            case 'rest':
                state.value.restSec += deltaSec;
                restStreakSec.value += deltaSec;
                breakStreakSec.value = 0;
                if (restStreakSec.value >= 45 * 60) {
                    state.value.continuousDrivingSec = 0;
                }
                if (restStreakSec.value >= 11 * 3600) {
                    resetDailyCounters();
                    restStreakSec.value = 0;
                }
                break;
            default:
                break;
        }
    }

    function tick() {
        const now = Date.now();
        if (!lastTickMs.value) {
            lastTickMs.value = now;
            return;
        }

        const deltaSec = (now - lastTickMs.value) / 1000;
        lastTickMs.value = now;
        applyTick(deltaSec);
        persistState();
    }

    function selectMode(mode: TachographMode) {
        manualMode.value = mode;
        persistState();
    }

    onMounted(() => {
        hydrateState();
        lastTickMs.value = Date.now();
        timer = setInterval(tick, 1000);
    });

    onUnmounted(() => {
        if (timer) clearInterval(timer);
        timer = null;
    });
</script>

<template>
    <div class="widget-panel w-full px-4 py-3">
        <div class="flex items-center justify-between">
            <div class="text-xs text-slate-500 font-mono uppercase tracking-wider">{{ Locale('widgets.tachograph.name') }}</div>
            <div class="text-[10px] text-slate-400 font-mono">{{ activeMode }}</div>
        </div>

        <div class="mt-2 grid grid-cols-4 gap-2 text-center font-mono">
            <div>
                <div class="text-[10px] text-slate-500">{{ Locale('widgets.tachograph.driving') }}</div>
                <div class="text-sm font-black text-cyan-300">{{ drivingLabel }}</div>
            </div>
            <div>
                <div class="text-[10px] text-slate-500">{{ Locale('widgets.tachograph.work') }}</div>
                <div class="text-sm font-black text-sky-300">{{ workLabel }}</div>
            </div>
            <div>
                <div class="text-[10px] text-slate-500">{{ Locale('widgets.tachograph.break') }}</div>
                <div class="text-sm font-black text-emerald-300">{{ breakLabel }}</div>
            </div>
            <div>
                <div class="text-[10px] text-slate-500">{{ Locale('widgets.tachograph.rest') }}</div>
                <div class="text-sm font-black text-violet-300">{{ restLabel }}</div>
            </div>
        </div>

        <div class="mt-2 space-y-1.5">
            <div>
                <div class="flex justify-between text-[10px] text-slate-400 font-mono">
                    <span>{{ Locale('widgets.tachograph.continuous') }}</span>
                    <span>{{ continuousLabel }} / 04:30</span>
                </div>
                <div class="h-1.5 rounded bg-slate-800">
                    <div class="h-1.5 rounded transition-all" :class="needsBreak ? 'bg-rose-500' : 'bg-cyan-400'" :style="{ width: `${continuousPercent}%` }" />
                </div>
            </div>
            <div>
                <div class="flex justify-between text-[10px] text-slate-400 font-mono">
                    <span>{{ Locale('widgets.tachograph.daily_driving') }}</span>
                    <span>{{ drivingLabel }} / 09:00</span>
                </div>
                <div class="h-1.5 rounded bg-slate-800">
                    <div class="h-1.5 rounded transition-all" :class="drivingLimitReached ? 'bg-rose-500' : 'bg-emerald-500'" :style="{ width: `${drivingPercent}%` }" />
                </div>
            </div>
            <div>
                <div class="flex justify-between text-[10px] text-slate-400 font-mono">
                    <span>{{ Locale('widgets.tachograph.daily_work') }}</span>
                    <span>{{ workLabel }} / 13:00</span>
                </div>
                <div class="h-1.5 rounded bg-slate-800">
                    <div class="h-1.5 rounded bg-sky-500 transition-all" :style="{ width: `${workPercent}%` }" />
                </div>
            </div>
        </div>

        <div class="mt-2 flex flex-wrap gap-1.5">
            <button
                v-for="mode in (['auto', 'driving', 'work', 'break', 'rest'] as TachographMode[])"
                :key="mode"
                class="px-1.5 py-1 text-[10px] rounded border font-mono uppercase transition-colors"
                :class="manualMode === mode ? 'border-cyan-500/70 text-cyan-300 bg-cyan-500/20' : 'border-slate-700 text-slate-400 hover:text-slate-200'"
                @click="selectMode(mode)"
            >
                {{ Locale(`widgets.tachograph.${mode}`) }}
            </button>
        </div>
    </div>
</template>
