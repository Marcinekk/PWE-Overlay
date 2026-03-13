interface AppSettings {
    speedUnit: 'kmh' | 'mph';
    language: 'pl' | 'en';
    telemetryInterval: number;
    showGrid: boolean;
    accentColor: string;
    overlayGlobalOpacity: number;
    useMetricSystem: boolean;
    nightMode: boolean;
}

const defaultSettings: AppSettings = {
    speedUnit: 'kmh',
    language: 'pl',
    telemetryInterval: 50,
    showGrid: false,
    accentColor: '#f59e0b',
    overlayGlobalOpacity: 90,
    useMetricSystem: true,
    nightMode: true,
}

export type { AppSettings };
export { defaultSettings };