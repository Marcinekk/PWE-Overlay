<script setup lang="ts">
    import { Locale } from '@composables/useLanguage';
    import { faTruck, faWrench } from '@fortawesome/free-solid-svg-icons';

    import type { TelemetryData } from '@interfaces/telemetry';

    const props = defineProps<{
        activeTab: string;
        telemetry: TelemetryData;
        connected: boolean;
    }>();
</script>

<template>
    <section v-show="activeTab === 'truck'" key="truck" class="pm2__grid">
        <div class="pm2__card hud-glass pm2__card--wide">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faTruck" />
                <span>{{ Locale('pauseMenu2.truck.title') }}</span>
            </div>
            <div class="pm2__kv pm2__kv--cols">
                <div><span>{{ Locale('pauseMenu2.truck.model') }}</span><strong>{{ telemetry.truckMake }} {{ telemetry.truckModel }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.plate') }}</span><strong>{{ telemetry.truckLicensePlate }}</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.odometer') }}</span><strong>{{ Math.round(telemetry.odometer).toLocaleString() }} km</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.fuel') }}</span><strong>{{ Math.round(telemetry.fuel) }} / {{ Math.round(telemetry.fuelMax) }} L</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.adblue') }}</span><strong>{{ Math.round(telemetry.adblue) }} / {{ Math.round(telemetry.adblueMax) }} L</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.wear') }}</span><strong>{{ Math.round((1 - telemetry.truckWear) * 100) }}%</strong></div>
            </div>
        </div>

        <div class="pm2__card hud-glass">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faWrench" />
                <span>{{ Locale('pauseMenu2.truck.components') }}</span>
            </div>
            <div class="pm2__kv">
                <div><span>{{ Locale('pauseMenu2.truck.engine') }}</span><strong>{{ Math.round((1 - telemetry.engineWear) * 100) }}%</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.transmission') }}</span><strong>{{ Math.round((1 - telemetry.transmissionWear) * 100) }}%</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.cabin') }}</span><strong>{{ Math.round((1 - telemetry.cabinWear) * 100) }}%</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.chassis') }}</span><strong>{{ Math.round((1 - telemetry.chassisWear) * 100) }}%</strong></div>
                <div><span>{{ Locale('pauseMenu2.truck.wheels') }}</span><strong>{{ Math.round((1 - telemetry.wheelsWear) * 100) }}%</strong></div>
                <div v-if="telemetry.trailerAttached"><span>{{ Locale('pauseMenu2.truck.trailer') }}</span><strong>{{ Math.round((1 - telemetry.trailerWear) * 100) }}%</strong></div>
            </div>
        </div>
    </section>
</template>