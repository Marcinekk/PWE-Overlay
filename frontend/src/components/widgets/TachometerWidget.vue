<script setup lang="ts">
    import { computed } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';

    const telemetry = useTelemetryStore();
    const rpmDisplay = computed(() => Math.round(telemetry.data.rpm / 100));
    const rpmPct = computed(() => telemetry.rpmPercent / 100);

    const rpmColor = computed(() => {
        const p = rpmPct.value;
        if (p > 0.9) return '#ff4f49';
        if (p > 0.75) return '#ffd563';
        return '#5ed4ff';
    });

    const rpmAngle = computed(() => 220 + rpmPct.value * 260);

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

    const ticks = computed(() => {
        const result: any[] = [];
        const cx = 100, cy = 110, rOuter = 93, rInner = 85, rLabel = 72;
        const toRad = (d: number) => (d * Math.PI) / 180;
        const maxRpm = telemetry.data.rpmMax;
        const steps = 20;

        for (let i = 0; i <= steps; i++) {
            const angle = 220 + (i / steps) * 260;
            const rad = toRad(angle);
            const major = i % 4 === 0;
            const ri = major ? rInner - 4 : rInner;
            const val = Math.round((i / steps) * (maxRpm / 100));
            result.push({
                x1: cx + rOuter * Math.cos(rad),
                y1: cy + rOuter * Math.sin(rad),
                x2: cx + ri * Math.cos(rad),
                y2: cy + ri * Math.sin(rad),
                major,
                label: val,
                lx: cx + rLabel * Math.cos(rad),
                ly: cy + rLabel * Math.sin(rad),
                angle
            });
        }
        return result;
    })
</script>

<template>
    <div class="widget-panel w-full h-full flex items-center justify-center">
        <svg viewBox="0 0 200 200" class="w-full h-full">
            <path :d="arcPath(220, 320, 85)" fill="none" stroke="#1f2937" stroke-width="14" stroke-linecap="round" />
            <path
                :d="arcPath(220, rpmAngle, 85)"
                fill="none"
                :stroke="rpmColor"
                stroke-width="14"
                stroke-linecap="round"
                class="transition-all duration-75"
                style="filter: drop-shadow(0 0 5px currentColor);"
            />

            <g v-for="(tick, i) in ticks" :key="i">
                <line
                    :x1="tick.x1" :y1="tick.y1"
                    :x2="tick.x2" :y2="tick.y2"
                    :stroke="tick.angle >= 220 + 0.85 * 260 ? '#ff4f49' : (tick.major ? '#94a3b8' : '#334155')"
                    :stroke-width="tick.major ? 2 : 1"
                />

                <text
                    v-if="tick.major"
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
                x="100"
                y="100"
                fill="#e6edf7"
                font-size="36"
                font-weight="700"
                text-anchor="middle"
                dominant-baseline="middle"
                font-family="'JetBrains Mono', monospace"
            >
                {{ rpmDisplay }}
            </text>

            <text
                x="100"
                y="128"
                fill="#8f9db1"
                font-size="9"
                text-anchor="middle"
                font-family="'JetBrains Mono', monospace"
            >
                × 100 RPM
            </text>

            <text x="100" y="158" :fill="telemetry.data.engineOn ? '#6ce0a0' : '#64748b'" font-size="9" text-anchor="middle" font-family="'JetBrains Mono', monospace">
                {{ telemetry.data.engineOn ? '● ON' : '○ OFF' }}
            </text>
        </svg>
    </div>
</template>
