<script setup lang="ts">
    import { ref, watch, onMounted } from 'vue';
    import { usePluginBridgeStore } from '@stores/pluginBridge';
    import { useModal } from '@composables/useModal';
    import { useLogbookStore } from '@stores/logbook';
    import { useSettingsStore } from '@stores/settings';
    import { Locale } from '@composables/useLanguage';
    import { faBook, faLocationDot, faRoute, faInfoCircle } from '@fortawesome/free-solid-svg-icons';

    import JobView from '@components/ui/JobView.vue';
    import ConfirmModal from '@components/ui/ConfirmModal.vue';

    import type { TelemetryData } from '@interfaces/telemetry';

    const props = defineProps<{
        activeTab: string;
        telemetry: TelemetryData;
        connected: boolean;
    }>();

    const bridge = usePluginBridgeStore();
    const modal = useModal();
    const logbookStore = useLogbookStore();
    const settings = useSettingsStore();
    const isLoading = ref(false);

    function fetchJobs() {
        if (!props.connected) return;
        isLoading.value = true;
        bridge.sendCommand('logbook/get', { limit: 50 });
    }
    
    watch(() => props.activeTab, (newTab) => {
        if (newTab === 'logbook') fetchJobs();
    });

    onMounted(() => {
        if (props.activeTab === 'logbook') fetchJobs();
    });

    function formatTime(ts: number): string {
        if (!ts) return '--';
        return new Date(ts).toLocaleString(undefined, { 
            day: '2-digit', 
            month: '2-digit', 
            year: 'numeric',
            hour: '2-digit', 
            minute: '2-digit' 
        });
    }

    function formatAmount(amount: number): string {
        return `${amount.toLocaleString()}$`;
    }

    function showDetails(job: any) {
        modal.open(JobView, { title: Locale('pauseMenu2.overview.job'), job: job, closeText: Locale('pauseMenu2.back') });
    }

    function showTruncateConfirm() {
        modal.open(ConfirmModal, {
            title: Locale('pauseMenu2.logbook.truncateConfirmTitle'),
            message: Locale('pauseMenu2.logbook.truncateConfirmMessage'),
            confirmText: Locale('pauseMenu2.logbook.truncateConfirmDelete'),
            cancelText: Locale('pauseMenu2.logbook.truncateConfirmCancel'),
            onConfirm: () => {
                bridge.sendCommand('logbook/truncate');
                setTimeout(() => fetchJobs(), 200);
                modal.close();
            }
        });
    }
</script>

<template>
    <section v-show="activeTab === 'logbook'" key="logbook" class="pm2__grid">
        <div class="pm2__card hud-glass pm2__card--wide flex flex-col">
            <div class="pm2__card-head shrink-0">
                <FontAwesomeIcon :icon="faBook" />
                <span>{{ Locale('pauseMenu2.tabs.logbook') }}</span>

                <button @click="showTruncateConfirm" class="pm2__btn pm2__btn--small pm2__btn--danger" v-if="connected && logbookStore.items.length > 0">
                    {{ Locale('pauseMenu2.logbook.truncate') }}
                </button>
            </div>

            <div class="flex-1 pr-2 max-h-[calc(100vh-203px)] custom-scrollbar overflow-y-auto">
                <div v-if="logbookStore.items.length === 0" class="flex flex-col items-center justify-center py-12 opacity-30">
                    <FontAwesomeIcon :icon="faRoute" class="text-4xl mb-4" />
                    <p>{{ connected ? Locale('pauseMenu2.logbook.noJobs') : Locale('pauseMenu2.logbook.notConnected') }}</p>
                </div>

                <div v-else class="space-y-3">
                    <div 
                        v-for="job in logbookStore.items" 
                        :key="job.id"
                        class="group p-4 rounded-xl border border-white/5 bg-white/5 hover:bg-white/10 hover:border-white/10 transition-all cursor-pointer relative overflow-hidden"
                        @click="showDetails(job)"
                    >
                        <div 
                            class="absolute left-0 top-0 bottom-0 w-1"
                            :class="job.status === 'delivered' ? 'bg-emerald-500' : 'bg-rose-500'"
                        ></div>

                        <div class="flex flex-col md:flex-row md:items-center justify-between gap-4">
                            <div class="flex-1 min-w-0">
                                <div class="flex items-center gap-2 mb-1">
                                    <span class="text-xs font-bold uppercase tracking-wider px-2 py-0.5 rounded bg-white/10">
                                        {{ job.cargo }}
                                    </span>
                                    <span class="text-xs opacity-40 font-mono">{{ formatTime(job.timestamp) }}</span>
                                </div>

                                <div class="flex items-center gap-3 text-sm">
                                    <div class="flex items-center gap-1.5 min-w-0">
                                        <FontAwesomeIcon :icon="faLocationDot" class="text-rose-400 shrink-0" />
                                        <span class="truncate">{{ job.sourceCity }}</span>
                                    </div>
                                    <FontAwesomeIcon :icon="faRoute" class="opacity-20 text-xs" />
                                    <div class="flex items-center gap-1.5 min-w-0">
                                        <FontAwesomeIcon :icon="faLocationDot" class="text-emerald-400 shrink-0" />
                                        <span class="truncate">{{ job.destinationCity }}</span>
                                    </div>
                                </div>
                            </div>

                            <div class="flex items-center gap-6 shrink-0 font-mono">
                                <div class="text-right">
                                    <div class="text-[10px] opacity-40 uppercase tracking-tighter">{{ Locale('pauseMenu2.logbook.income') }}</div>
                                    <div class="text-emerald-400 font-bold">{{ formatAmount(job.income) }}</div>
                                </div>
                                <div class="text-right">
                                    <div class="text-[10px] opacity-40 uppercase tracking-tighter">{{ Locale('pauseMenu2.logbook.distance') }}</div>
                                    <div class="font-bold">{{ job.distance.toFixed(0) }} {{ settings.settings.speedUnit === 'kmh' ? 'km' : 'mi' }}</div>
                                </div>
                                <div class="flex items-center justify-center w-8 h-8 rounded-full bg-white/5 group-hover:bg-white/10">
                                    <FontAwesomeIcon :icon="faInfoCircle" class="opacity-40 group-hover:opacity-100 transition-opacity" />
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </section>
</template>

<style scoped>
    .custom-scrollbar::-webkit-scrollbar {
        width: 4px;
    }
    .custom-scrollbar::-webkit-scrollbar-track {
        background: rgba(255, 255, 255, 0.02);
        border-radius: 10px;
    }
    .custom-scrollbar::-webkit-scrollbar-thumb {
        background: rgba(255, 255, 255, 0.1);
        border-radius: 10px;
    }
    .custom-scrollbar::-webkit-scrollbar-thumb:hover {
        background: rgba(255, 255, 255, 0.2);
    }
</style>