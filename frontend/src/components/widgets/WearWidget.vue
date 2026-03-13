<script setup lang="ts">
    import { useTelemetryStore } from '@stores/telemetry';
    import WearBar from '@components/ui/WearBar.vue';
    import { faWrench } from '@fortawesome/free-solid-svg-icons';

    const telemetry = useTelemetryStore();
</script>

<template>
    <div class="widget-panel w-full h-full px-4 py-3 overflow-auto">
        <div class="flex items-center gap-2 mb-3">
            <FontAwesomeIcon :icon="faWrench" class="text-cyan-300 text-sm" />
            <span class="text-xs text-slate-400 uppercase tracking-wider font-mono">Stan techniczny</span>
        </div>

        <div class="space-y-2">
            <WearBar label="Silnik" :wear="telemetry.data.engineWear" />
            <WearBar label="Skrzynia" :wear="telemetry.data.transmissionWear" />
            <WearBar label="Kabina" :wear="telemetry.data.cabinWear" />
            <WearBar label="Podwozie" :wear="telemetry.data.chassisWear" />
            <WearBar label="Koła" :wear="telemetry.data.wheelsWear" />

            <div v-if="telemetry.data.trailerAttached" class="border-t border-slate-700/60 pt-2">
                <WearBar label="Naczepa" :wear="telemetry.data.trailerWear" />
            </div>
        </div>
    </div>
</template>
