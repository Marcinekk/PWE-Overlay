import { ref, onUnmounted } from 'vue';
import { useTelemetryStore } from '@stores/telemetry';

function useSimulator() {
    const store = useTelemetryStore();
    const running = ref(false);
    let interval: ReturnType<typeof setInterval> | null = null;

    let simSpeed = 0;
    let simRpm = 800;
    let simGear = 0;
    let simFuel = 450;
    let simTime = 480;
    let blinkerTick = 0;
    let phase = 0;

    function tick() {
        simTime += 0.0167;
        blinkerTick++;

        if (phase === 0) {
            simGear = 0;
            simSpeed = Math.max(0, simSpeed - 2);
            simRpm = Math.max(600, simRpm - 50);
            if (simSpeed <= 0 && Math.random() < 0.01) phase = 1;
        } else if (phase === 1) {
            if (simGear === 0) simGear = 1;
            simSpeed = Math.min(90, simSpeed + 0.8);
            simRpm = Math.min(2200, 800 + simSpeed * 15);
            simGear = Math.floor(simSpeed / 15) + 1;
            simFuel -= 0.002;
            if (simSpeed >= 88 && Math.random() < 0.02) phase = 2;
        } else if (phase === 2) {
            simSpeed += (Math.random() - 0.5) * 0.5;
            simSpeed = Math.max(82, Math.min(95, simSpeed));
            simRpm = 1500 + simSpeed * 5;
            simFuel -= 0.001;
            if (Math.random() < 0.005) phase = 3;
        } else if (phase === 3) {
            simSpeed = Math.max(0, simSpeed - 1.5);
            simRpm = Math.max(600, simRpm - 80);
            simGear = Math.max(0, Math.floor(simSpeed / 15));
            if (simSpeed <= 0) phase = 0;
        }

        const blinkerOn = blinkerTick % 20 < 10;
        const isBraking = phase === 3;

        store.update({
            speed: Math.max(0, simSpeed),
            speedLimit: 90,
            rpm: simRpm,
            rpmMax: 2500,
            gear: simGear,
            gearDashboard: simGear,
            engineOn: phase !== 0 || simSpeed > 0,
            electricEnabled: true,
            fuel: simFuel,
            fuelMax: 600,
            fuelAvgConsumption: 32,
            fuelRange: (simFuel / 32) * 100,
            cruiseControl: phase === 2,
            cruiseControlSpeed: phase === 2 ? 90 : 0,
            truckWear: 0.05,
            engineWear: 0.03,
            transmissionWear: 0.04,
            cabinWear: 0.02,
            chassisWear: 0.03,
            wheelsWear: 0.06,
            trailerAttached: true,
            trailerWear: 0.08,
            trailerMass: 24000,
            trailerName: 'Schmitz Cargobull',
            trailerCargoMass: 18000,
            jobIncome: 12500,
            jobRemainingTime: 7200,
            jobSourceCity: 'Warszawa',
            jobSourceCompany: 'Agro Transport',
            jobDestinationCity: 'Berlin',
            jobDestinationCompany: 'Schenker GmbH',
            jobDistance: 580,
            jobDistanceRemaining: 320,
            jobCargoName: 'Zboże',
            navDistance: 320000,
            navTime: 10800,
            navSpeedLimit: 90,
            blinkerLeft: phase === 3 ? blinkerOn : false,
            blinkerRight: false,
            lightsBeamLow: true,
            lightsParking: false,
            lightsBeamHigh: false,
            lightsBeacon: false,
            lightsBrake: isBraking,
            lightsReverse: false,
            parkBrake: phase === 0 && simSpeed <= 0,
            motorBrake: false,
            fuelWarning: simFuel < 50,
            airPressureWarning: false,
            airPressureEmergency: false,
            adblueWarning: false,
            oilPressureWarning: false,
            waterTemperatureWarning: false,
            batteryVoltageWarning: false,
            airPressure: 8.5,
            oilPressure: 45,
            oilTemperature: 90,
            waterTemperature: 85,
            batteryVoltage: 27.5,
            adblue: 42,
            adblueMax: 60,
            gameTime: Math.floor(simTime),
            truckMake: 'Volvo',
            truckModel: 'FH16 750',
            truckLicensePlate: 'WA 1337 TS',
            truckLicensePlateCountry: 'pl',
            odometer: 125430 + Math.floor(simTime),
            accelerationX: (Math.random() - 0.5) * 0.1,
            accelerationY: 0,
            accelerationZ: isBraking ? -0.3 : phase === 1 ? 0.2 : 0,
        });
    }

    function start() {
        if (running.value) return;
        running.value = true;
        store.setConnected(true);
        interval = setInterval(tick, 100);
    }

    function stop() {
        running.value = false;
        store.setConnected(false);
        if (interval) {
            clearInterval(interval);
            interval = null;
        }
    }

    onUnmounted(() => stop());
    return { running, start, stop };
}

export { useSimulator };