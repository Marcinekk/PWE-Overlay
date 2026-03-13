<script setup lang="ts">
    import { computed } from 'vue';

    const props = defineProps<{ label: string; wear: number }>();
    const healthPct = computed(() => Math.round((1 - props.wear) * 100));

    const barColor = computed(() => {
        const h = healthPct.value;
        if (h < 30) return 'bg-red-500';
        if (h < 60) return 'bg-yellow-500';
        return 'bg-green-500';
    });

    const textColor = computed(() => {
        const h = healthPct.value;
        if (h < 30) return 'text-red-400';
        if (h < 60) return 'text-yellow-400';
        return 'text-green-400';
    })
</script>

<template>
    <div class="flex items-center gap-2">
        <span class="text-xs text-slate-500 font-mono w-16 shrink-0">{{ label }}</span>
        <div class="flex-1 h-1.5 bg-slate-800 rounded-full overflow-hidden">
            <div
                class="h-full rounded-full transition-all duration-300"
                :class="barColor"
                :style="{ width: `${healthPct}%` }"
            />
        </div>
        <span class="text-xs font-mono w-8 text-right" :class="textColor">{{ healthPct }}%</span>
    </div>
</template>