<script setup lang="ts">
    import { useRouter } from 'vue-router';
    import { useTelemetryStore } from '@stores/telemetry';
    import { useLayoutStore } from '@stores/layout';
    import { useSettingsStore } from '@stores/settings';

    import DraggableWidget from '@components/layout/DraggableWidget.vue';
    import SettingsPanel from '@components/layout/SettingsPanel.vue';
    import { widgetDefinitions } from '@widgets/index';

    import { faArrowsUpDownLeftRight, faCheck, faGear } from '@fortawesome/free-solid-svg-icons';

    const isDev = import.meta.env.DEV;
    const telemetry = useTelemetryStore();
    const layout = useLayoutStore();
    const settings = useSettingsStore();
    const router = useRouter();

    function openPauseMenu() {
        router.push('/pause-menu');
    }
</script>

<template>
    <div
        class="overlay-root select-none"
        :style="{ opacity: settings.settings.overlayGlobalOpacity / 100 }"
        :class="isDev ? 'bg-slate-950' : 'bg-transparent'"
    >
        <div
            v-if="!telemetry.connected"
            class="fixed top-2 left-1/2 -translate-x-1/2 flex items-center gap-2 bg-red-500/20 border border-red-500/55 text-red-300 text-xs font-mono px-3 py-1.5 rounded-full z-50 backdrop-blur-sm"
        >
            <div class="w-1.5 h-1.5 rounded-full bg-red-400 animate-pulse" />
            Brak połączenia
        </div>

        <Transition name="fade">
            <div
                v-if="layout.editMode"
                class="fixed top-2 left-1/2 -translate-x-1/2 flex items-center gap-3 hud-glass hud-text text-xs font-mono px-4 py-1.5 rounded-full z-50"
            >
                <FontAwesomeIcon :icon="faArrowsUpDownLeftRight" />
                <span>Tryb edycji</span>

                <button @click="layout.toggleEditMode()" class="ml-2 hud-accent-cyan hover:text-slate-100 transition-colors">
                    <FontAwesomeIcon :icon="faCheck" />
                    Gotowe
                </button>
            </div>
        </Transition>

        <DraggableWidget
            v-for="definition in widgetDefinitions"
            :id="definition.meta.name"
            :key="definition.meta.name"
        >
            <component :is="definition.component" />
        </DraggableWidget>

        <button
            v-if="telemetry?.data?.webViewFocus"
            class="cursor-pointer fixed top-3 right-3 z-1999 w-9 h-9 rounded-full hud-glass text-slate-300 hover:text-cyan-300 hover:border-cyan-400/55 transition-all flex items-center justify-center"
            :class="{ 'text-cyan-300 border-cyan-400/55': layout.editMenu }"
            @click="openPauseMenu"
        >
            <FontAwesomeIcon :icon="faGear" :class="{ 'animate-spin-slow': layout.editMenu }" />
        </button>

        <SettingsPanel
            :open="layout.editMenu"
            @close="layout.editMenu = false"
        />

        <div v-if="layout?.editMenu" class="fixed inset-0 z-1998" @click="layout.editMenu = false" />
    </div>
</template>