<script setup lang="ts">
    import { useModal } from '@composables/useModal';
    const { isOpen, currentModal, modalProps, close } = useModal();
</script>

<template>
    <transition
        enter-active-class="animate__animated animate__fadeIn animate__faster"
        leave-active-class="animate__animated animate__fadeOut animate__faster"
    >
        <div
            v-if="isOpen"
            class="fixed inset-0 z-100 flex items-center justify-center pointer-events-auto"
        >
            <div class="absolute inset-0 bg-slate-950/55 backdrop-blur-sm" @click="close"></div>
            <transition
                enter-active-class="animate__animated animate__zoomIn animate__faster"
                leave-active-class="animate__animated animate__zoomOut animate__faster"
            >
                <div
                    v-if="currentModal"
                    class="relative z-10 w-full max-w-2xl mx-auto p-4"
                >
                    <component :is="currentModal" v-bind="modalProps" @close="close" />
                </div>
            </transition>
        </div>
    </transition>
</template>