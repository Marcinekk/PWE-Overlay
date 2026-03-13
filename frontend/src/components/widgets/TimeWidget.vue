<script setup lang="ts">
    import { ref, onMounted, onUnmounted } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { Locale } from '@composables/useLanguage';

    import { faClock } from '@fortawesome/free-solid-svg-icons';

    const telemetry = useTelemetryStore();
    const realTime = ref('');

    function updateTime() {
        const now = new Date();
        realTime.value = now.toLocaleTimeString('pl-PL', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
    }

    let timer: ReturnType<typeof setInterval>;

    onMounted(() => { updateTime(); timer = setInterval(updateTime, 1000) });
    onUnmounted(() => clearInterval(timer));
</script>

<template>
    <div class="widget-panel w-full px-4 py-3">
        <div class="flex items-center justify-between">
            <div class="flex items-center gap-2">
                <FontAwesomeIcon :icon="faClock" class="text-cyan-300 text-sm" />
                <span class="text-xs text-slate-400 uppercase tracking-wider font-mono">{{ Locale('widgets.time.name') }}</span>
            </div>
        </div>

        <div class="flex items-center justify-between mt-0.5">
            <div>
                <div class="text-xs text-slate-500 font-mono">{{ Locale('widgets.time.game') }}</div>
                <div class="text-xl font-black font-mono text-cyan-300 leading-none">{{ telemetry.gameTimeFormatted }}</div>
            </div>

            <div class="h-8 w-px bg-slate-600/60" />

            <div class="text-right">
                <div class="text-xs text-slate-500 font-mono">{{ Locale('widgets.time.local') }}</div>
                <div class="text-xl font-black font-mono text-slate-100 leading-none">{{ realTime }}</div>
            </div>
        </div>
    </div>
</template>