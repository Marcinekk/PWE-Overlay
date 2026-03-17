import { defineStore } from 'pinia';
import { ref } from 'vue';

export interface LogbookEntry {
    id: number;
    timestamp: number;
    cargo: string;
    sourceCity: string;
    sourceCompany: string;
    destinationCity: string;
    destinationCompany: string;
    income: number;
    xp: number;
    distance: number;
    status: 'delivered' | 'cancelled';
    cargo_unit_mass: number;
    is_special: boolean;
}

export const useLogbookStore = defineStore('logbook', () => {
    const items = ref<LogbookEntry[]>([]);

    function setAll(data: LogbookEntry[]) {
        items.value = data;
    }

    return { items, setAll };
});
