<script setup lang="ts">
    import { onMounted, onUnmounted } from 'vue';
    import { useRouter, useRoute } from 'vue-router';
    import { useTelemetry } from '@composables/useTelemetry';
    import Modal from '@components/Modal.vue';

    const router = useRouter();
    const route = useRoute();

    useTelemetry();

    function handleKeydown(e: KeyboardEvent) {
        if (e.key === 'Escape') {
            e.preventDefault();
            if (route.path === '/pause-menu') {
                router.push('/');
            } else {
                router.push('/pause-menu');
            }
        }
    }

    onMounted(() => window.addEventListener('keydown', handleKeydown));
    onUnmounted(() => window.removeEventListener('keydown', handleKeydown));
</script>

<template>
    <router-view />
    <Modal />
</template>