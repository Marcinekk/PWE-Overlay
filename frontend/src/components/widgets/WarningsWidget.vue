<script setup lang="ts">
    import { computed, ref, onMounted, onUnmounted } from "vue";
    import { useTelemetryStore } from "@stores/telemetry";
    import { faWind, faTriangleExclamation, faGasPump, faDroplet, faOilCan, faTemperatureHigh, faBatteryQuarter } from "@fortawesome/free-solid-svg-icons";
    import { Locale } from "@composables/useLanguage";

    const telemetry = useTelemetryStore();
    const blink = ref(true);
    let blinkTimer: ReturnType<typeof setInterval>;
    onMounted(() => {
        blinkTimer = setInterval(() => {
            blink.value = !blink.value;
        }, 600);
    });
    onUnmounted(() => clearInterval(blinkTimer));

    const indicators = computed(() => {
        const t = telemetry.data;
        return [
            {
                key: "airEmergency",
                icon: faTriangleExclamation,
                label: Locale('widgets.warnings.airEmergency'),
                active: t.airPressureEmergency,
                level: "critical",
            },
            {
                key: "airWarning",
                icon: faWind,
                label: Locale('widgets.warnings.airWarning'),
                active: t.airPressureWarning && !t.airPressureEmergency,
                level: "warning",
            },
            {
                key: "fuel",
                icon: faGasPump,
                label: Locale('widgets.warnings.fuel'),
                active: t.fuelWarning,
                level: "warning",
            },
            {
                key: "adblue",
                icon: faDroplet,
                label: Locale('widgets.warnings.adblue'),
                active: t.adblueWarning,
                level: "warning",
            },
            {
                key: "oil",
                icon: faOilCan,
                label: Locale('widgets.warnings.oil'),
                active: t.oilPressureWarning,
                level: "critical",
            },
            {
                key: "water",
                icon: faTemperatureHigh,
                label: Locale('widgets.warnings.water'),
                active: t.waterTemperatureWarning,
                level: "critical",
            },
            {
                key: "battery",
                icon: faBatteryQuarter,
                label: Locale('widgets.warnings.battery'),
                active: t.batteryVoltageWarning,
                level: "warning",
            },
        ];
    });
    //
    const activeIndicators = computed(() => indicators.value.filter((i) => i.active));
    const hasAny = computed(() => activeIndicators.value.length > 0);
</script>

<template>
    <div
        v-if="hasAny"
        class="w-full h-full px-3 py-2 flex items-center"
    >
        <div class="flex flex-wrap gap-2 items-center">
            <div
                v-for="ind in activeIndicators"
                class="warning-light"
                :key="ind.key"
                :title="ind.label"
                :class="{
                    'warning-light--critical': ind.level === 'critical',
                    'warning-light--warning': ind.level === 'warning',
                    'warning-light--blink': ind.level === 'critical' && !blink,
                }"
            >
                <FontAwesomeIcon :icon="ind.icon" class="warning-light__icon" />
                <span class="warning-light__label">{{ ind.label }}</span>
            </div>
        </div>
    </div>
</template>

<style scoped>
    .warning-light {
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        gap: 2px;
        width: 52px;
        height: 52px;
        border-radius: 8px;
        border: 1px solid transparent;
        transition: opacity 0.15s ease;
        cursor: default;
        user-select: none;
    }

    .warning-light--warning {
        background: rgba(234, 88, 12, 0.15);
        border-color: rgba(234, 88, 12, 0.5);
        color: #fb923c;
        box-shadow:
            0 0 8px rgba(234, 88, 12, 0.25),
            inset 0 0 6px rgba(234, 88, 12, 0.08);
    }

    .warning-light--critical {
        background: rgba(220, 38, 38, 0.18);
        border-color: rgba(220, 38, 38, 0.6);
        color: #f87171;
        box-shadow:
        0 0 10px rgba(220, 38, 38, 0.3),
        inset 0 0 8px rgba(220, 38, 38, 0.1);
    }

    .warning-light--blink {
        opacity: 0.25;
    }

    .warning-light__icon {
        font-size: 1.1rem;
    }

    .warning-light__label {
        font-size: 0.55rem;
        font-family: "Courier New", monospace;
        font-weight: 700;
        letter-spacing: 0.05em;
        text-transform: uppercase;
        line-height: 1;
    }
</style>
