<script setup lang="ts">
    defineProps<{
        title: string;
        message: string;
        confirmText: string;
        cancelText: string;
    }>();

    const emit = defineEmits(['confirm', 'close']);
</script>

<template>
    <div class="select-none hud-glass p-6 rounded-lg border border-white/10 shadow-2xl flex flex-col gap-4 max-w-md mx-auto animate__animated animate__fadeInUp animate__faster">
        <div class="flex flex-col gap-1">
            <h3 class="text-xl font-bold uppercase tracking-wider text-white/90">{{ title }}</h3>
            <div class="h-px w-full bg-white/10 mb-2"></div>
            <p class="text-sm text-white/60 font-mono leading-relaxed">{{ message }}</p>
        </div>

        <div class="flex justify-end gap-3 mt-2">
            <button 
                class="pm2__btn pm2__btn--ghost px-6"
                @click="emit('close')"
            >
                {{ cancelText }}
            </button>
            <button 
                class="pm2__btn border-rose-500/50 text-rose-400 bg-rose-500/10 px-6 hover:bg-rose-500/20 transition-colors"
                @click="emit('confirm')"
            >
                {{ confirmText }}
            </button>
        </div>
    </div>
</template>
