import { defineStore } from 'pinia';
import { ref } from 'vue';
import type { Transaction } from '@interfaces/transaction';
import { usePluginBridgeStore } from '@stores/pluginBridge';

export const useTransactionsStore = defineStore('transactions', () => {
    const items = ref<Transaction[]>([]);
    const bridge = usePluginBridgeStore();

    function setAll(data: Transaction[]) {
        items.value = data;
    }

    function updateStatus(id: number, status: Transaction['status']) {
        const tx = items.value.find(t => t.id === id);
        if (tx) tx.status = status;
    }

    function approve(id: number) {
        bridge.sendCommand('transactions/approve', { id });
        updateStatus(id, 'approved');
    }

    function reject(id: number) {
        bridge.sendCommand('transactions/reject', { id });
        updateStatus(id, 'rejected');
    }

    function fetchAll(statusFilter?: string) {
        bridge.sendCommand('transactions/get', {
            status: statusFilter ?? '',
            limit: 200,
        });
    }

    function fetchManualApprove() {
        bridge.sendCommand('transactions/get_manual_approve', {});
    }

    function truncateTransactions() {
        bridge.sendCommand('transactions/truncate', {});
    }

    return { items, setAll, updateStatus, approve, reject, fetchAll, fetchManualApprove, truncateTransactions };
});
