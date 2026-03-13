<script setup lang="ts">
    import { ref, watch, onMounted } from 'vue';
    import { useEventsStore, type PendingEvents } from '@stores/events';
    import { useLayoutStore } from '@stores/layout';
    import { faWallet, faArrowTrendUp, faArrowTrendDown, faCircleInfo } from '@fortawesome/free-solid-svg-icons';
    import { Locale } from '@composables/useLanguage';

    const layout = useLayoutStore();
    const eventsStore = useEventsStore();
    const currentEvent = ref<PendingEvents | null>(null);
    const visible = ref(false);
    const isProcessing = ref(false);

    const descriptions = [
        { name: 'player_driver_income', label: Locale('widgets.alerts.player_driver_income'), },
        { name: 'player_job_income', label: Locale('widgets.alerts.player_job_income'), },
        { name: 'player_truck_sold', label: Locale('widgets.alerts.player_truck_sold'), },
        { name: 'player_loan_taken', label: Locale('widgets.alerts.player_loan_taken'), },
        { name: 'player_truck_sold_exchange', label: '', ignore: true },

        { name: 'player_fined', label: Locale('widgets.alerts.player_fined'), },
        { name: 'player_refuel', label: Locale('widgets.alerts.player_refuel'), },
        { name: 'player_emergency_refuel', label: Locale('widgets.alerts.player_emergency_refuel'), },

        { name: 'player_service_fix', label: Locale('widgets.alerts.player_service_fix'), },
        { name: 'player_service_mod', label: Locale('widgets.alerts.player_service_mod'), },
        { name: 'player_service_renovation', label: Locale('widgets.alerts.player_service_renovation'), },
        { name: 'player_service_teleport', label: Locale('widgets.alerts.player_service_teleport'), },
        { name: 'player_service_renovation', label: Locale('widgets.alerts.player_service_renovation'), },

        { name: 'player_job_cancelled', label: Locale('widgets.alerts.player_job_cancelled'), },

        { name: 'player_truck_bought', label: Locale('widgets.alerts.player_truck_bought'), },
        { name: 'player_truck_bought_used_online', label: Locale('widgets.alerts.player_truck_bought_used_online'), },
        
        { name: 'player_garage_teleport', label: Locale('widgets.alerts.player_garage_teleport'), },
        { name: 'player_garage_upgrade', label: Locale('widgets.alerts.player_garage_upgrade'), },
        { name: 'player_garage_bought', label: Locale('widgets.alerts.player_garage_bought'), },
        { name: 'player_garage_bought_online', label: Locale('widgets.alerts.player_garage_bought_online'), },

        { name: 'player_quick_travel', label: Locale('widgets.alerts.player_quick_travel'), },

        { name: 'player_driver_hired', label: Locale('widgets.alerts.player_driver_hired'), },

        { name: 'player_loan_paid', label: Locale('widgets.alerts.player_loan_paid'), },
        { name: 'player_loan_paid_one', label: Locale('widgets.alerts.player_loan_paid_one'), },
        
        { name: 'player_tollgate_paid', label: Locale('widgets.alerts.player_tollgate_paid'), },
    ];
    
    const types = [
        { name: 'bank', label: Locale('widgets.alerts.bank'), },
        { name: 'xp', label: Locale('widgets.alerts.xp'), },
    ];
    
    function getLabel(name: any) {
        const description = descriptions.find(desc => desc.name === name);
        return description ? description.label : name;
    }
    
    function getType(type: any) {
        const typeLabel = types.find(types => types.name === type);
        return typeLabel ? typeLabel.label : type;
    }
    
    function getIcon(type: any) {
        if(!type || type === 'bank') return faWallet;
        return faCircleInfo;
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
                :class="currentEvent?.income ? 'border-l-emerald-400' : 'border-l-rose-500'"
            >
                <div class="bg-slate-900/40 backdrop-blur-md px-4 py-3 rounded-xl flex items-center gap-4">
                    <div 
                        class="w-12 h-12 rounded-full flex items-center justify-center text-xl shrink-0"
                        :class="currentEvent?.income ? 'bg-emerald-500/20 text-emerald-400' : 'bg-rose-500/20 text-rose-400'"
                    >
                        <FontAwesomeIcon :icon="currentEvent?.income ? faArrowTrendUp : faArrowTrendDown" />
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
                            :class="currentEvent?.income === 'true' ? 'text-emerald-400' : 'text-rose-400'"
                        >
                            {{ currentEvent?.income === 'true' ? '+' : '-' }}{{ (currentEvent?.amount || 0).toLocaleString() }}$
                        </div>
                    </div>
                    
                    <div class="opacity-10 absolute -right-2.5 -bottom-2.5 scale-150 rotate-[-15deg]">
                        <FontAwesomeIcon :icon="getIcon(currentEvent?.type)" class="text-6xl" />
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