<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { useSettingsStore } from '@stores/settings';
    import { Locale } from '@composables/useLanguage';

    const telemetry = useTelemetryStore();
    const settings = useSettingsStore();

    const unit = computed(() => settings.settings.speedUnit === 'kmh' ? 'km/h' : 'mph');
    const maxSpeed = computed(() => settings.settings.speedUnit === 'kmh' ? 160 : 100);

    const speedDisplay = computed(() => {
        const spd = telemetry.data.speed;
        return Math.round(settings.settings.speedUnit === 'kmh' ? spd : spd * 0.621371);
    });

    const speedLimitDisplay = computed(() => {
        const lim = telemetry.data.speedLimit;
        return Math.round(settings.settings.speedUnit === 'kmh' ? lim : lim * 0.621371);
    });

    const isSpeeding = computed(() => telemetry.isSpeeding);

    function arcPath(startDeg: number, endDeg: number, r: number) {
        const cx = 100, cy = 110;
        const toRad = (d: number) => (d * Math.PI) / 180;
        const x1 = cx + r * Math.cos(toRad(startDeg));
        const y1 = cy + r * Math.sin(toRad(startDeg));
        const x2 = cx + r * Math.cos(toRad(endDeg));
        const y2 = cy + r * Math.sin(toRad(endDeg));
        const sweep = endDeg - startDeg;
        const large = sweep > 180 ? 1 : 0;

        if (Math.abs(sweep) < 0.5) return '';
        return `M ${x1} ${y1} A ${r} ${r} 0 ${large} 1 ${x2} ${y2}`;
    }

    const speedAngle = computed(() => {
        const pct = Math.min(speedDisplay.value / maxSpeed.value, 1);
        return 220 + pct * 260;
    });

    const speedLimitAngle = computed(() => {
        const pct = Math.min(speedLimitDisplay.value / maxSpeed.value, 1);
        return 220 + pct * 260;
    });

    const ticks = computed(() => {
        const result: any[] = [];
        const cx = 100, cy = 110, rOuter = 93, rInner = 85, rLabel = 72;
        const toRad = (d: number) => (d * Math.PI) / 180;
        const steps = 32;

        for (let i = 0; i <= steps; i++) {
            const angle = 220 + (i / steps) * 260;
            const rad = toRad(angle);
            const major = i % 4 === 0;
            const ri = major ? rInner - 4 : rInner;
            const speedVal = Math.round((i / steps) * maxSpeed.value);

            result.push({
                x1: cx + rOuter * Math.cos(rad),
                y1: cy + rOuter * Math.sin(rad),
                x2: cx + ri * Math.cos(rad),
                y2: cy + ri * Math.sin(rad),
                major,
                label: major ? speedVal : undefined,
                lx: cx + rLabel * Math.cos(rad),
                ly: cy + rLabel * Math.sin(rad)
            });
        }
        return result;
    });
</script>

<template>
    <div class="widget-panel w-full h-full flex items-center justify-center relative">
        <svg viewBox="0 0 200 200" class="w-full h-full">
            <path
                :d="arcPath(220, 320, 85)"
                fill="none"
                stroke="#1f2937"
                stroke-width="14"
                stroke-linecap="round"
            />

            <path
                v-if="telemetry.data.speedLimit > 0"
                :d="arcPath(220, speedLimitAngle, 85)"
                fill="none"
                stroke="#ff4f4930"
                stroke-width="14"
                stroke-linecap="round"
            />

            <path
                :d="arcPath(220, speedAngle, 85)"
                fill="none"
                :stroke="isSpeeding ? '#ff4f49' : '#5ed4ff'"
                stroke-width="14"
                stroke-linecap="round"
                class="transition-all duration-100"
                style="filter: drop-shadow(0 0 6px currentColor);"
            />

            <g v-for="(tick, i) in ticks" :key="i">
                <line
                    :x1="tick.x1" :y1="tick.y1"
                    :x2="tick.x2" :y2="tick.y2"
                    :stroke="tick.major ? '#94a3b8' : '#334155'"
                    :stroke-width="tick.major ? 2 : 1"
                />

                <text
                    v-if="tick.major && tick.label !== undefined"
                    :x="tick.lx" :y="tick.ly"
                    fill="#94a3b8"
                    font-size="9"
                    text-anchor="middle"
                    dominant-baseline="middle"
                    font-family="'JetBrains Mono', monospace"
                >
                    {{ tick.label }}
                </text>
            </g>

            <text
                x="100" y="108"
                fill="#e6edf7"
                font-size="44"
                font-weight="700"
                text-anchor="middle"
                dominant-baseline="middle"
                font-family="'JetBrains Mono', monospace"
                :class="{ 'fill-red-400': isSpeeding }"
            >
                {{ speedDisplay }}
            </text>

            <text
                x="100" y="138"
                fill="#8f9db1"
                font-size="11"
                text-anchor="middle"
                font-family="'JetBrains Mono', monospace"
            >
                {{ unit }}
            </text>

            <g v-if="telemetry.data.cruiseControl">
                <circle cx="45" cy="155" r="14" fill="#083426" stroke="#6ce0a0" stroke-width="1" />
                <text x="45" y="155" fill="#6ce0a0" font-size="9" text-anchor="middle" dominant-baseline="middle">{{ Locale('widgets.speedometer.cruise') }}</text>
            </g>
        </svg>
    </div>
</template>