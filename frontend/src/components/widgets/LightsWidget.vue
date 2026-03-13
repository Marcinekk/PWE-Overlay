<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';

    const telemetry = useTelemetryStore();
    const lights = computed(() => [
        {
            key: 'park',
            abbr: 'P',
            label: 'Parking',
            active: telemetry.data.lightsParking,
            level: 'warning',
        },
        {
            key: 'low',
            abbr: 'LB',
            label: 'Światła mijania',
            active: telemetry.data.lightsBeamLow,
            level: 'info',
        },
        {
            key: 'high',
            abbr: 'HB',
            label: 'Światła drogowe',
            active: telemetry.data.lightsBeamHigh,
            level: 'info',
        },
        {
            key: 'beacon',
            abbr: 'BK',
            label: 'Kogut',
            active: telemetry.data.lightsBeacon,
            level: 'warning',
        },
    ]);
</script>

<template>
    <div class="w-full h-full px-3 py-2 flex items-center gap-2 select-none">

        <!-- Left blinker -->
        <div
            class="blinker-arrow blinker-arrow--left"
            :class="{ 'blinker-arrow--active': telemetry.data.blinkerLeft }"
            title="Kierunkowskaz lewy"
        >◄</div>

        <!-- Light indicators -->
        <div class="flex gap-2 items-center flex-1">
            <div
                v-for="light in lights"
                :key="light.key"
                :title="light.label"
                class="light-indicator"
                :class="{
                    'light-indicator--active-warning': light.active && light.level === 'warning',
                    'light-indicator--active-info':    light.active && light.level === 'info',
                    'light-indicator--inactive':       !light.active,
                }"
            >
                {{ light.abbr }}
            </div>
        </div>

        <!-- Right blinker -->
        <div
            class="blinker-arrow blinker-arrow--right"
            :class="{ 'blinker-arrow--active': telemetry.data.blinkerRight }"
            title="Kierunkowskaz prawy"
        >►</div>

    </div>
</template>

<style scoped>
    .blinker-arrow {
        font-size: 1.25rem;
        line-height: 1;
        transition: color 0.1s ease;
        color: #1e293b;
    }
    .blinker-arrow--active {
        color: #4ade80;
        text-shadow: 0 0 8px rgba(74, 222, 128, 0.6);
    }

    .light-indicator {
        display: flex;
        align-items: center;
        justify-content: center;
        width: 36px;
        height: 36px;
        border-radius: 6px;
        border: 1px solid transparent;
        font-size: 0.65rem;
        font-family: 'Courier New', monospace;
        font-weight: 700;
        letter-spacing: 0.04em;
        text-transform: uppercase;
        transition: all 0.15s ease;
        cursor: default;
    }

    .light-indicator--inactive {
        background: rgba(15, 23, 42, 0.6);
        border-color: rgba(51, 65, 85, 0.5);
        color: #334155;
    }

    .light-indicator--active-warning {
        background: rgba(234, 88, 12, 0.15);
        border-color: rgba(234, 88, 12, 0.5);
        color: #fb923c;
        box-shadow:
            0 0 8px rgba(234, 88, 12, 0.25),
            inset 0 0 6px rgba(234, 88, 12, 0.08);
    }

    .light-indicator--active-info {
        background: rgba(6, 182, 212, 0.12);
        border-color: rgba(6, 182, 212, 0.5);
        color: #67e8f9;
        box-shadow:
            0 0 8px rgba(6, 182, 212, 0.25),
            inset 0 0 6px rgba(6, 182, 212, 0.08);
    }
</style>
