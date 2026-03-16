import { defineStore } from 'pinia';
import { ref } from 'vue';
import { defaults, type MiscSettings } from '@interfaces/misc';

export const useMiscStore = defineStore('misc', () => {
    const miscSettings = ref<MiscSettings>(structuredClone(defaults));

    function set(key: string, value: any) {
        if (miscSettings.value.hasOwnProperty(key))
            (miscSettings.value as any)[key] = value;
    }

    function update(payload: Partial<MiscSettings>) {
        for (const key in payload) {
            if (payload.hasOwnProperty(key)) (miscSettings.value as any)[key] = payload[key as keyof MiscSettings];
        }
    }

    function get(key: string) {
        if (miscSettings.value.hasOwnProperty(key))
            return (miscSettings.value as any)[key];
        return null;
    }

    return { miscSettings, update, set, get };
});