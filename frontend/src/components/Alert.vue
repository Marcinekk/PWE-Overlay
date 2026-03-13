<script setup lang="ts">
    import { computed } from 'vue';
    import { toast } from 'vue-sonner';
    import { faXmark, faCircleInfo, faCircleCheck, faCircleXmark, faCircleExclamation } from '@fortawesome/free-solid-svg-icons';

    const props = defineProps<{
        title?: string;
        message?: string;
        type?: 'info' | 'success' | 'warning' | 'error';
    }>();

    const variantConfig = computed(() => {
        switch (props.type) {
            case 'success':
                return { icon: faCircleCheck, classes: 'bg-green-500/20 text-green-400 border-green-500/30' };
            case 'error':
                return { icon: faCircleXmark, classes: 'bg-red-500/20 text-red-400 border-red-500/30' };
            case 'warning':
                return { icon: faCircleExclamation, classes: 'bg-yellow-500/20 text-yellow-300 border-yellow-500/30' };
            case 'info':
            default:
                return { icon: faCircleInfo, classes: 'bg-cyan-500/20 text-cyan-300 border-cyan-500/30' };
        }
    });
</script>

<template>
    <div class="pointer-events-auto shadow-2xl animate__animated animate__fadeInUp animate__faster w-full max-w-sm">
        <div class="rounded-2xl border border-slate-500/30 bg-slate-950/80 backdrop-blur-xl p-4 shadow-xl relative overflow-hidden">
            <div
                class="absolute top-0 left-0 w-full h-1 opacity-50"
                :class="variantConfig.classes.split(' ')[0]"
            ></div>

            <button
                class="absolute text-slate-400 right-4 top-4 hover:text-slate-100 transition-colors p-1 rounded-full hover:bg-white/10"
                @click="toast.dismiss()"
            >
                <FontAwesomeIcon :icon="faXmark" size="sm" />
            </button>

            <div class="flex items-start gap-4 mt-1">
                <div
                    class="flex items-center justify-center w-10 h-10 rounded-xl shrink-0 shadow-lg border"
                    :class="variantConfig.classes"
                >
                    <FontAwesomeIcon :icon="variantConfig.icon" size="lg" />
                </div>

                <div class="flex flex-col pr-6">
                    <h5
                        v-if="props.title"
                        class="text-base font-semibold text-slate-100 tracking-wide"
                    >
                        {{ props.title }}
                    </h5>

                    <p
                        v-if="props.message"
                        class="text-sm text-slate-300/80"
                        :class="{ 'mt-1': props.title }"
                    >
                        {{ props.message }}
                    </p>
                </div>
            </div>
        </div>
    </div>
</template>