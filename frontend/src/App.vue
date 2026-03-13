<script setup lang="ts">
    import { useTelemetry } from '@composables/useTelemetry';
    useTelemetry();
</script>

<template>
    <router-view />
</template>