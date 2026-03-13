import { ref, computed } from 'vue';
import { defineStore } from 'pinia';
import type { TelemetryData } from '@interfaces/telemetry';
import { defaultTelemetry } from '@interfaces/telemetry';

const useTelemetryStore = defineStore('telemetry', () => {
    const data = ref<TelemetryData>({ ...defaultTelemetry });
    const connected = ref(false);
    const lastUpdate = ref<Date | null>(null);

    const speedKmh = computed(() => Math.round(data.value.speed));
    const speedPercent = computed(() => Math.min((data.value.speed / 150) * 100, 100));
    const rpmPercent = computed(() => (data.value.rpm / data.value.rpmMax) * 100);
    const fuelPercent = computed(() => (data.value.fuel / data.value.fuelMax) * 100);
    const adbluePercent = computed(() => (data.value.adblue / data.value.adblueMax) * 100);

    const gearLabel = computed(() => {
        const g = data.value.gear;
        if (g === 0) return 'N';
        if (g < 0) return 'R';
        return String(g);
    });

    const isSpeeding = computed(() => data.value.speedLimit > 0 && data.value.speed > data.value.speedLimit + 2);

    const gameTimeFormatted = computed(() => {
        const minutes = data.value.gameTime % (24 * 60);
        const h = Math.floor(minutes / 60).toString().replace(/^(\d)$/, '0$1');
        const m = Math.floor(minutes % 60).toString().replace(/^(\d)$/, '0$1');
        return `${h}:${m}`;
    });

    const hasAnyWarning = computed(() =>
        data.value.airPressureWarning ||
        data.value.airPressureEmergency ||
        data.value.fuelWarning ||
        data.value.adblueWarning ||
        data.value.oilPressureWarning ||
        data.value.waterTemperatureWarning ||
        data.value.batteryVoltageWarning
    );

    const navDistanceFormatted = computed(() => {
        const d = data.value.navDistance;
        if (d >= 1000) return `${(d / 1000).toFixed(1)} km`;
        return `${Math.round(d)} m`;
    });

    const navTimeFormatted = computed(() => {
        const s = data.value.navTime;
        const h = Math.floor(s / 3600);
        const m = Math.floor((s % 3600) / 60);
        if (h > 0) return `${h}h ${m}min`;
        return `${m} min`;
    });

    const jobRemainingFormatted = computed(() => {
        const s = data.value.jobRemainingTime;
        if (s <= 0) return '--:--';
        const h = Math.floor(s / 3600);
        const m = Math.floor((s % 3600) / 60);
        return `${h}h ${m < 10 ? '0' + m : m}min`;
    });

    const truckWearPercent = computed(() => Math.round((1 - data.value.truckWear) * 100));

    function update(payload: Partial<TelemetryData>) {
        for (const key in payload) {
            if (payload.hasOwnProperty(key)) (data.value as any)[key] = payload[key as keyof TelemetryData];
        }
        lastUpdate.value = new Date();
        connected.value = true;
    }

    function setConnected(v: boolean) {
        connected.value = v;
    }

    return {
        data,
        connected,
        lastUpdate,
        speedKmh,
        speedPercent,
        rpmPercent,
        fuelPercent,
        adbluePercent,
        gearLabel,
        isSpeeding,
        gameTimeFormatted,
        hasAnyWarning,
        navDistanceFormatted,
        navTimeFormatted,
        jobRemainingFormatted,
        truckWearPercent,
        update,
        setConnected,
    };
});

export { useTelemetryStore };