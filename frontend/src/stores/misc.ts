import { defineStore } from 'pinia';
import { ref } from 'vue';

export interface MiscSettings {
    isMultiplayer: boolean;
    gameMismatch: boolean;
    frameworkMismatch: boolean;
}

export const useMiscStore = defineStore('misc', () => {
    const miscSettings = ref<MiscSettings>({
        isMultiplayer: false,
        gameMismatch: false,
        frameworkMismatch: false,
    });

    function setMultiplayer(v: boolean) {
        miscSettings.value.isMultiplayer = v;
    }

    function setGameMismatch(v: boolean) {
        miscSettings.value.gameMismatch = v;
    }

    function setFrameworkMismatch(v: boolean) {
        miscSettings.value.frameworkMismatch = v;
    }

    return { miscSettings, setMultiplayer, setGameMismatch, setFrameworkMismatch };
});