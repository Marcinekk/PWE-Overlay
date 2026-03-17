<script setup lang="ts">
    import type { LogbookEntry } from '@stores/logbook';
    import { useSettingsStore } from '@stores/settings';
    import { Locale } from '@composables/useLanguage';
    import { 
        faLocationDot, 
        faRoute, 
        faMoneyBillTrendUp, 
        faStar, 
        faRoad, 
        faCheckCircle, 
        faCircleXmark, 
        faClock,
        faCalendarDay,
        faBox,
        faWeight
    } from '@fortawesome/free-solid-svg-icons';

    const props = defineProps<{
        title: string;
        job: LogbookEntry;
        closeText: string;
    }>();

    const emit = defineEmits(['close']);
    const settings = useSettingsStore();

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

    function formatCurrency(amount: number): string {
        return amount.toLocaleString() + '$';
    }
</script>

<template>
    <div class="select-none hud-glass rounded-2xl border border-white/10 shadow-2xl flex flex-col max-w-2xl mx-auto overflow-hidden animate__animated animate__zoomIn animate__faster">
        <div class="p-6 bg-linear-to-r from-white/5 to-transparent border-b border-white/5 flex justify-between items-start">
            <div>
                <h3 class="text-xs font-bold uppercase tracking-[0.2em] text-white/40 mb-1">{{ title }}</h3>
                <h2 class="text-2xl font-black text-white leading-none tracking-tight">{{ job.cargo }}</h2>
            </div>
            <div 
                class="px-3 py-1 rounded-full text-[10px] font-bold uppercase tracking-wider"
                :class="job.status === 'delivered' ? 'bg-emerald-500/20 text-emerald-400 border border-emerald-500/30' : 'bg-rose-500/20 text-rose-400 border border-rose-500/30'"
            >
                <FontAwesomeIcon :icon="job.status === 'delivered' ? faCheckCircle : faCircleXmark" class="mr-1" />
                {{ job.status }}
            </div>
        </div>

        <div class="p-6 space-y-8">
            <div class="relative flex items-center justify-between px-4">
                <div class="z-10 bg-white/5 p-2 rounded-xl border border-white/10 flex flex-col items-center w-32">
                    <FontAwesomeIcon :icon="faLocationDot" class="text-rose-400 mb-1" />
                    <span class="text-[10px] text-white/40 uppercase font-bold tracking-tighter">{{ Locale('pauseMenu2.logbook.source') }}</span>
                    <span class="text-sm font-bold truncate w-full text-center text-white">{{ job.sourceCity }}</span>
                    <span class="text-[9px] text-white/20 truncate w-full text-center">{{ job.sourceCompany }}</span>
                </div>

                <div class="flex-1 h-px bg-linear-to-r from-rose-400/30 via-white/10 to-emerald-400/30 relative mx-2">
                    <div class="absolute inset-0 flex items-center justify-center">
                        <div class="w-8 h-8 rounded-full bg-white/5 border border-white/10 flex items-center justify-center transition-transform hover:scale-110 group">
                            <FontAwesomeIcon :icon="faRoute" class="text-xs text-white/20 group-hover:text-white/60 transition-colors" />
                        </div>
                    </div>
                </div>

                <div class="z-10 bg-white/5 p-2 rounded-xl border border-white/10 flex flex-col items-center w-32">
                    <FontAwesomeIcon :icon="faLocationDot" class="text-emerald-400 mb-1" />
                    <span class="text-[10px] text-white/40 uppercase font-bold tracking-tighter">{{ Locale('pauseMenu2.logbook.destination') }}</span>
                    <span class="text-sm font-bold truncate w-full text-center text-white">{{ job.destinationCity }}</span>
                    <span class="text-[9px] text-white/20 truncate w-full text-center">{{ job.destinationCompany }}</span>
                </div>
            </div>

            <div class="grid grid-cols-3 gap-3">
                <div class="bg-white/5 border border-white/5 p-4 rounded-2xl flex flex-col items-center transition-all hover:bg-white/10 hover:-translate-y-1">
                    <FontAwesomeIcon :icon="faMoneyBillTrendUp" class="text-emerald-400 mb-2 text-lg" />
                    <span class="text-[10px] font-bold uppercase tracking-widest text-white/20 mb-1">{{ Locale('pauseMenu2.overview.income') }}</span>
                    <span class="text-emerald-400">{{ formatCurrency(job.income) }}</span>
                </div>
                <div class="bg-white/5 border border-white/5 p-4 rounded-2xl flex flex-col items-center transition-all hover:bg-white/10 hover:-translate-y-1">
                    <FontAwesomeIcon :icon="faStar" class="text-blue-400 mb-2 text-lg" />
                    <span class="text-[10px] font-bold uppercase tracking-widest text-white/20 mb-1">XP</span>
                    <span class="text-blue-400">{{ job.xp }} XP</span>
                </div>
                <div class="bg-white/5 border border-white/5 p-4 rounded-2xl flex flex-col items-center transition-all hover:bg-white/10 hover:-translate-y-1">
                    <FontAwesomeIcon :icon="faRoad" class="text-amber-400 mb-2 text-lg" />
                    <span class="text-[10px] font-bold uppercase tracking-widest text-white/20 mb-1">{{ Locale('widgets.job.distance') }}</span>
                    <span class="text-lg font-black tracking-tight text-amber-400">{{ job.distance.toFixed(0) }} {{ settings.settings.speedUnit === 'kmh' ? 'km' : 'mi' }}</span>
                </div>
            </div>

            <div>
                <div class="flex items-center justify-between px-4 py-3 rounded-xl bg-white/5 border border-white/5 font-mono text-xs">
                    <div class="flex items-center gap-2">
                        <FontAwesomeIcon :icon="faBox" class="text-white/20" />
                        <span class="text-white/60">{{ job.cargo }}</span>
                    </div>
                    <div class="flex items-center gap-2">
                        <FontAwesomeIcon :icon="faWeight" class="text-white/20" />
                        <span class="text-white/40">{{ job.cargo_unit_mass }}</span>
                    </div>
                </div>

                <div class="mt-2 flex items-center justify-between px-4 py-3 rounded-xl bg-white/5 border border-white/5 font-mono text-xs">
                    <div class="flex items-center gap-2">
                        <FontAwesomeIcon :icon="faClock" class="text-white/20" />
                        <span class="text-white/60">{{ formatTime(job.timestamp) }}</span>
                    </div>
                    <div class="flex items-center gap-2">
                        <FontAwesomeIcon :icon="faCalendarDay" class="text-white/20" />
                        <span class="text-white/40">ID: #{{ job.id }}</span>
                    </div>
                </div>
            </div>
        </div>

        <div class="p-4 bg-black/20 border-t border-white/5 flex justify-end">
            <button 
                class="pm2__btn pm2__btn--ghost px-10 py-3 rounded-full font-bold uppercase tracking-widest text-xs transition-all hover:bg-white hover:text-black active:scale-95 shadow-xl"
                @click="emit('close')"
            >
                {{ closeText }}
            </button>
        </div>
    </div>
</template>

<style scoped>
    .stat-card:hover {
        box-shadow: 0 10px 20px -10px rgba(0,0,0,0.5);
    }
</style>
