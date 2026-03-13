import { defineStore } from 'pinia';
import { ref } from 'vue';

export interface PendingEvents {
    id: number;
    income: string;
    amount: number;
    description: string;
    timestamp?: number;
    type?: 'bank' | 'alert';
}

export const useEventsStore = defineStore('events', () => {
    const eventQueue = ref<PendingEvents[]>([]);

    function addEvent(event: Omit<PendingEvents, 'timestamp'>) {
        eventQueue.value.push({
            ...event,
            timestamp: Date.now(),
        });
    }

    function removeEvent(id: number) {
        eventQueue.value = eventQueue.value.filter(e => e.id !== id);
    }

    return { eventQueue, addEvent, removeEvent };
});