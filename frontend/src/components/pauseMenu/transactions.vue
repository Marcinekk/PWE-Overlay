<script setup lang="ts">
    import { ref, computed } from 'vue';
    import { Locale } from '@composables/useLanguage';
    import { useTransactionsStore } from '@stores/transactions';
    import { useMiscStore } from '@stores/misc';
    import { usePluginBridgeStore } from '@stores/pluginBridge';
    import { useModal } from '@composables/useModal';
    import { useDescriptions } from '@composables/useDescriptions';

    import ConfirmModal from '@components/ui/ConfirmModal.vue';
    import { faWallet, faArrowTrendUp, faArrowTrendDown, faRefresh, faCheck, faXmark, faCogs } from '@fortawesome/free-solid-svg-icons';

    import type { TelemetryData } from '@interfaces/telemetry';

    const miscStore = useMiscStore();
    const txStore = useTransactionsStore();
    const bridge = usePluginBridgeStore();
    const modal = useModal();
    const { getLabel } = useDescriptions();

    const statusFilter = ref<'all' | 'pending' | 'approved' | 'rejected'>('all');
    const filteredDbTransactions = computed(() => {
        if (statusFilter.value === 'all') return txStore.items;
        return txStore.items.filter(t => t.status === statusFilter.value);
    });

    function fetchTransactions() {
        txStore.fetchAll();
        txStore.fetchManualApprove();
    }

    function formatDbAmount(t: (typeof txStore.items)[0]): string {
        const prefix = t.income ? '+' : '-';
        return `${prefix}${(t.amount ?? 0).toLocaleString()}$`;
    }

    function formatDbTime(ts: number): string {
        if (!ts) return '--';
        return new Date(ts).toLocaleTimeString(undefined, { hour: '2-digit', minute: '2-digit', second: '2-digit' });
    }

    function setManualApprove() {
        const value = !miscStore.get('isManualTransactionApprove');
        bridge.sendCommand('transactions/set_manual_approve', { approve: value });
        miscStore.set('isManualTransactionApprove', value);
    }

    function showTruncateConfirm() {
        modal.open(ConfirmModal, {
            title: Locale('pauseMenu2.transactions.truncateConfirmTitle'),
            message: Locale('pauseMenu2.transactions.truncateConfirmMessage'),
            confirmText: Locale('pauseMenu2.transactions.truncateConfirmDelete'),
            cancelText: Locale('pauseMenu2.transactions.truncateConfirmCancel'),
            onConfirm: () => {
                txStore.truncateTransactions();
                setTimeout(() => fetchTransactions(), 200);
                modal.close();
            }
        });
    }

    const props = defineProps<{
        activeTab: string;
        telemetry: TelemetryData;
        connected: boolean;
    }>();
</script>

<template>
    <section v-show="activeTab === 'transactions'" key="transactions">
        <div class="pm2__card hud-glass mb-8">
            <div class="pm2__card-head">
                <FontAwesomeIcon :icon="faCogs" />
                <span>{{ Locale('pauseMenu2.transactions.settings') }}</span>
            </div>
            <div class="pm2__warnings">
                <div class="flex flex-row gap-8">
                    <span class="flex flex-row gap-2">
                        <div class="flex items-center h-5">
                            <input
                                id="manualApprove"
                                aria-describedby="manualApprove-text"
                                type="checkbox"
                                value=""
                                :checked="miscStore.get('isManualTransactionApprove')"
                                @change="setManualApprove()"
                                class="w-4 h-4 border border-default-medium rounded-xs bg-blue-500/15 text-blue-500 focus:ring-transparent"
                            />
                        </div>

                        <div class="ms-2 text-sm select-none">
                            <label for="manualApprove" class="py-6 opacity-80 text-base font-mono">{{ Locale('pauseMenu2.transactions.manualApprove') }}</label>
                            <p id="manualApprove-text" class="opacity-50 text-xs font-mono">{{ Locale('pauseMenu2.transactions.manualApproveDescription') }}</p>
                        </div>
                    </span>

                    <div class="w-px h-auto bg-gray-300/20"></div>

                    <span class="flex flex-row gap-2">
                        <div class="flex items-center h-5">
                            <button 
                                class="pm2__btn pm2__btn--ghost"
                                @click="showTruncateConfirm"
                            >
                                <FontAwesomeIcon :icon="faRefresh" />
                            </button>
                        </div>
                        <div class="ms-2 text-sm select-none">
                            <label for="truncateTransactions" class="py-6 opacity-80 text-base font-mono">{{ Locale('pauseMenu2.transactions.truncateTransactions') }}</label>
                            <p id="truncateTransactions-text" class="opacity-50 text-xs font-mono">{{ Locale('pauseMenu2.transactions.truncateTransactionsDescription') }}</p>
                        </div>
                    </span>

                </div>
            </div>
        </div>

        <div class="pm2__card hud-glass">
            <div class="pm2__lang">
                <div class="pm2__lang-head">
                    <div class="pm2__lang-title">
                        <FontAwesomeIcon :icon="faWallet" />
                        <span>{{ Locale('pauseMenu2.transactions.title') }}</span>
                    </div>
                    <div class="flex flex-wrap gap-2.5">
                        <button 
                            class="pm2__btn"
                            :class="statusFilter==='all' ? 'pm2__btn--primary' : 'pm2__btn--ghost'"
                            @click="statusFilter='all'"
                        >
                            {{ Locale('pauseMenu2.transactions.filterAll') }}
                        </button>

                        <button
                            class="pm2__btn"
                            :class="statusFilter==='pending' ? 'pm2__btn--primary' : 'pm2__btn--ghost'"
                            @click="statusFilter='pending'"
                        >
                            {{ Locale('pauseMenu2.transactions.filterPending') }}
                        </button>

                        <button
                            class="pm2__btn"
                            :class="statusFilter==='approved' ? 'border-emerald-500/50 text-emerald-400 bg-emerald-500/10' : 'pm2__btn--ghost'"
                            @click="statusFilter='approved'"
                        >
                            {{ Locale('pauseMenu2.transactions.filterApproved') }}
                        </button>

                        <button
                            class="pm2__btn"
                            :class="statusFilter==='rejected' ? 'border-rose-500/50 text-rose-400 bg-rose-500/10' : 'pm2__btn--ghost'"
                            @click="statusFilter='rejected'"
                        >
                            {{ Locale('pauseMenu2.transactions.filterRejected') }}
                        </button>

                        <button
                            class="pm2__btn pm2__btn--ghost"
                            @click="fetchTransactions"
                        >
                            <FontAwesomeIcon :icon="faRefresh" />
                        </button>
                    </div>
                </div>

                <div v-if="filteredDbTransactions.length === 0" class="text-center py-6 opacity-40 text-xs font-mono">
                    {{ Locale('pauseMenu2.transactions.empty') }}
                </div>

                <div v-else class="flex flex-col gap-2 max-h-105 overflow-y-auto">
                    <div
                        v-for="t in filteredDbTransactions"
                        :key="t.id"
                        class="flex items-center justify-between p-2.5 rounded-lg border border-white/6 bg-black/18"
                    >
                        <div class="flex items-center gap-2.5">
                            <div
                                class="flex items-center justify-center w-7.5 h-7.5 rounded-full shrink-0"
                                :class="t.income ? 'bg-emerald-500/15 text-emerald-400' : 'bg-rose-500/15 text-rose-400'"
                            >
                                <FontAwesomeIcon :icon="t.income ? faArrowTrendUp : faArrowTrendDown" />
                            </div>
                            <div class="flex flex-col gap-0.5">
                                <span class="text-xs opacity-50 uppercase tracking-wider">{{ getLabel(t.type) }}</span>
                            </div>
                        </div>

                        <div class="flex items-center gap-2">
                            <div class="text-center">
                                <div class="text-xs opacity-40 font-mono">{{ formatDbTime(t.timestamp) }}</div>
                            </div>
                            <div class="text-right">
                                <div class="font-mono font-bold" :class="t.income ? 'text-emerald-400' : 'text-rose-400'">{{ formatDbAmount(t) }}</div>
                            </div>

                            <span
                                class="text-xs px-2 py-1 rounded-full uppercase tracking-wider"
                                :class="t.status==='approved' ? 'bg-emerald-500/15 text-emerald-400' : t.status==='rejected' ? 'bg-rose-500/15 text-rose-400' : 'bg-slate-500/12 text-slate-400'"
                            >
                                {{ t.status }}
                            </span>

                            <template v-if="t.status === 'pending'">
                                <button
                                    class="pm2__btn border-emerald-500/50 text-emerald-400 bg-emerald-500/10"
                                    @click="txStore.approve(t.id)"
                                >
                                    <FontAwesomeIcon :icon="faCheck" />
                                </button>

                                <button
                                    class="pm2__btn border-rose-500/50 text-rose-400 bg-rose-500/10"
                                    @click="txStore.reject(t.id)"
                                >
                                    <FontAwesomeIcon :icon="faXmark" />
                                </button>
                            </template>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </section>
</template>