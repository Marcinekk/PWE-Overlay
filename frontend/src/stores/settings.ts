import { ref } from 'vue';
import { defineStore } from 'pinia';
import type { AppSettings } from '@interfaces/settings';
import { defaultSettings } from '@interfaces/settings';

const STORAGE_KEY = 'pwe-settings';

const useSettingsStore = defineStore('settings', () => {
    const settings = ref<AppSettings>(load());

    function load(): AppSettings {
        try {
            const saved = localStorage.getItem(STORAGE_KEY);
            if (saved) return { ...defaultSettings, ...JSON.parse(saved) };
        } catch {

        }
        return { ...defaultSettings };
    }

    function save() {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(settings.value));
    }

    function update(patch: Partial<AppSettings>) {
        settings.value = { ...settings.value, ...patch };
        save();
    }

    function reset() {
        settings.value = { ...defaultSettings };
        save();
    }

    return { settings, update, save, reset };
});

export { useSettingsStore };