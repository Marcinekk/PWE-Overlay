<script setup lang="ts">
    import { ref, watch, onMounted } from 'vue';
    import { useEventsStore, type PendingEvents } from '@stores/events';
    import { useLayoutStore } from '@stores/layout';
    import { useTelemetryStore } from '@stores/telemetry';
    import { faWallet, faArrowTrendUp, faArrowTrendDown, faCircleInfo, faGasPump, faWrench, faTruck, faWarehouse, faCoins, faReceipt } from '@fortawesome/free-solid-svg-icons';
    import { useDescriptions } from '@composables/useDescriptions';

    const layout = useLayoutStore();
    const eventsStore = useEventsStore();
    const telemetry = useTelemetryStore();
    const currentEvent = ref<PendingEvents | null>(null);
    const { descriptions, getLabel, getType } = useDescriptions();
    const visible = ref(false);
    const isProcessing = ref(false);
    
    function getIcon(event: PendingEvents | null) {
        if (!event) return faWallet;
        
        switch (event.description) {
            case 'player_fined': return faReceipt;
            case 'player_refuel':
            case 'player_emergency_refuel': return faGasPump;
            case 'player_service_fix':
            case 'player_service_mod':
            case 'player_service_renovation': return faWrench;
            case 'player_truck_bought':
            case 'player_truck_bought_used_online': return faTruck;
            case 'player_garage_bought':
            case 'player_garage_bought_online':
            case 'player_garage_upgrade':
            case 'player_garage_teleport': return faWarehouse;
            case 'player_tollgate_paid': return faCoins;
            case 'player_loan_taken':
            case 'player_loan_paid':
            case 'player_loan_paid_one': return faWallet;
            default: return event.type === 'bank' ? faWallet : faCircleInfo;
        }
    }

    function getColorClass(event: PendingEvents | null, element: 'border' | 'iconBg' | 'text' | 'textAlt') {
        const isIncome = event?.income === 'true';
        let colorKey = isIncome ? 'emerald' : 'rose';

        if (!isIncome && event) {
            switch (event.description) {
                case 'player_fined': 
                case 'player_tollgate_paid': colorKey = 'amber'; break;
                case 'player_refuel':
                case 'player_emergency_refuel': colorKey = 'orange'; break;
                case 'player_service_fix':
                case 'player_service_mod':
                case 'player_service_renovation': colorKey = 'yellow'; break;
                case 'player_truck_bought':
                case 'player_truck_bought_used_online': colorKey = 'blue'; break;
                case 'player_garage_bought':
                case 'player_garage_bought_online':
                case 'player_garage_upgrade': colorKey = 'blue'; break;
            }
        }
        
        switch (colorKey) {
            case 'amber':
                if (element === 'border') return 'border-l-amber-400';
                if (element === 'iconBg') return 'bg-amber-500/20 text-amber-400';
                if (element === 'text') return 'text-amber-400';
                break;
            case 'orange':
                if (element === 'border') return 'border-l-orange-400';
                if (element === 'iconBg') return 'bg-orange-500/20 text-orange-400';
                if (element === 'text') return 'text-orange-400';
                break;
            case 'yellow':
                if (element === 'border') return 'border-l-yellow-400';
                if (element === 'iconBg') return 'bg-yellow-500/20 text-yellow-400';
                if (element === 'text') return 'text-yellow-400';
                break;
            case 'blue':
                if (element === 'border') return 'border-l-blue-400';
                if (element === 'iconBg') return 'bg-blue-500/20 text-blue-400';
                if (element === 'text') return 'text-blue-400';
                break;
            case 'emerald':
                if (element === 'border') return 'border-l-emerald-400';
                if (element === 'iconBg') return 'bg-emerald-500/20 text-emerald-400';
                if (element === 'text') return 'text-emerald-400';
                break;
            case 'rose':
            default:
                if (element === 'border') return 'border-l-rose-500';
                if (element === 'iconBg') return 'bg-rose-500/20 text-rose-400';
                if (element === 'text') return 'text-rose-400';
                break;
        }
        
        if (element === 'textAlt') return isIncome ? '+' : '-';
        return '';
    }
    
    async function processQueue() {
        if (isProcessing.value || eventsStore.eventQueue.length === 0 || layout.editMode) return;
        
        const event = eventsStore.eventQueue[0];
        const description = descriptions.find(desc => desc.name === event?.description);
        if (description) {
            if (description.ignore) {
                if(event) eventsStore.removeEvent(event?.id);
                return;
            }
            
            if(currentEvent.value) currentEvent.value.description = description.label;
        }
        
        isProcessing.value = true;
        if(event) currentEvent.value = event;
        
        visible.value = true;
        await new Promise(resolve => setTimeout(resolve, 4000));
        
        visible.value = false;
        await new Promise(resolve => setTimeout(resolve, 6000));
        
        if(event) eventsStore.removeEvent(event.id);
        
        currentEvent.value = null;
        isProcessing.value = false;
        
        processQueue();
    }
    
    watch(() => eventsStore.eventQueue.length, (newLen) => {
        if (newLen > 0 && !isProcessing.value) processQueue();
    });
    
    onMounted(() => {
        if (eventsStore.eventQueue.length > 0) processQueue();
    });
</script>

<template>
    <div class="fixed top-20 left-1/2 -translate-x-1/2 z-50 pointer-events-none min-w-[320px]">
        <Transition
            enter-active-class="animate__animated animate__backInDown"
            leave-active-class="animate__animated animate__backOutUp"
        >
            <div
                v-if="(visible && currentEvent) || layout.editMode"
                class="hud-glass p-1 rounded-2xl border-l-4 shadow-2xl overflow-hidden"
                :class="getColorClass(currentEvent, 'border')"
            >
                <div class="bg-slate-900/40 backdrop-blur-md px-4 py-3 rounded-xl flex items-center gap-4">
                    <div 
                        class="w-12 h-12 rounded-full flex items-center justify-center text-xl shrink-0"
                        :class="getColorClass(currentEvent, 'iconBg')"
                    >
                        <FontAwesomeIcon :icon="currentEvent?.income === 'true' ? faArrowTrendUp : faArrowTrendDown" />
                    </div>
                    
                    <div class="flex-1">
                        <div class="text-[10px] uppercase tracking-wider font-bold opacity-50 flex justify-between items-center">
                            <span class="text-white">{{ getType(currentEvent?.type) }}</span>
                        </div>
                        <div class="text-sm font-semibold text-slate-100 truncate max-w-50">
                            {{ getLabel(currentEvent?.description) }}
                        </div>
                        <div 
                            class="text-lg font-black font-mono tracking-tight"
                            :class="getColorClass(currentEvent, 'text')"
                        >
                            {{ getColorClass(currentEvent, 'textAlt') }}{{ (currentEvent?.amount || 0).toLocaleString() }}{{ telemetry?.data?.gameName === 'ATS' ? '$' : '€' }}
                        </div>
                    </div>
                    
                    <div class="opacity-10 absolute -right-2.5 -bottom-2.5 scale-150 rotate-[-15deg]">
                        <FontAwesomeIcon :icon="getIcon(currentEvent)" class="text-6xl" />
                    </div>
                </div>
            </div>
        </Transition>
    </div>
</template>

<style scoped>
    .hud-glass {
        background: linear-gradient(135deg, rgba(15, 23, 42, 0.8) 0%, rgba(30, 41, 59, 0.7) 100%);
        box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.5);
        border: 1px solid rgba(255, 255, 255, 0.08);
    }
</style>