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
    const transactionHistory = ref<PendingEvents[]>([]);

    function addEvent(event: Omit<PendingEvents, 'timestamp'>) {
        const newEvent = {
            ...event,
            timestamp: Date.now(),
        };
        eventQueue.value.push(newEvent);
        transactionHistory.value.unshift(newEvent);

        if (transactionHistory.value.length > 100) transactionHistory.value.pop();
    }

    function removeEvent(id: number) {
        eventQueue.value = eventQueue.value.filter(e => e.id !== id);
    }

    return { eventQueue, transactionHistory, addEvent, removeEvent };
});