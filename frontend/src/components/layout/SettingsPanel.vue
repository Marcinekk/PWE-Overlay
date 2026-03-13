<script setup lang="ts">
    import { computed, reactive, watch } from 'vue';
    import { useTelemetryStore } from '@stores/telemetry';
    import { useLayoutStore } from '@stores/layout';
    import { useSettingsStore } from '@stores/settings';
    import { useSimulator } from '@composables/useSimulator';
    import { widgetDefinitions } from '@widgets/index';

    import { faSliders, faXmark, faArrowsUpDownLeftRight, faRotateLeft, faFloppyDisk, faLock, faLockOpen } from '@fortawesome/free-solid-svg-icons';

    defineProps<{ open: boolean }>();
    const emit = defineEmits<{ close: [] }>();

    const telemetry = useTelemetryStore();
    const layout = useLayoutStore();
    const settingsStore = useSettingsStore();
    const simulator = useSimulator();
    const settingsLocal = reactive({ ...settingsStore.settings });
    const widgetsWithMeta = computed(() => {
        return widgetDefinitions.map((definition) => {
            const meta = definition.meta;
            const config = layout.getWidget(meta.name);
            return {
                id: meta.name,
                visible: config?.visible ?? (meta.defaultVisible ?? true),
                locked: config?.locked ?? (meta.defaultLocked ?? false),
                meta,
            };
        });
    });
    const hiddenWidgetsCount = computed(() => widgetsWithMeta.value.filter((widget) => !widget.visible).length);

    watch(() => settingsStore.settings, (s) => {
        for (const key in s) {
            if (s.hasOwnProperty(key)) {
                (settingsLocal as any)[key] = s[key as keyof typeof s];
            }
        }
    }, { deep: true });

    function saveSettings() {
        settingsStore.update({ ...settingsLocal });
        emit('close');
    }

    function confirmReset() {
        if (confirm('Reset układu do domyślnego?')) layout.resetLayout();
    }
</script>

<template>
    <Transition
        enter-active-class="animate__animated animate__slideInRight animate__faster"
        leave-active-class="animate__animated animate__slideOutRight animate__faster"
    >
        <div
            v-if="open"
            class="fixed right-0 top-0 h-full w-80 hud-glass-strong z-2000 flex flex-col overflow-hidden"
            data-no-drag
        >
            <div class="flex items-center justify-between px-4 py-3 border-b border-slate-600/40 bg-slate-900/65">
                <div class="flex items-center gap-2">
                    <FontAwesomeIcon :icon="faSliders" class="text-cyan-300" />
                    <span class="text-sm font-bold text-slate-100 font-mono">Ustawienia</span>
                </div>
                <button @click="$emit('close')" class="text-slate-400 hover:text-slate-100 transition-colors cursor-pointer">
                    <FontAwesomeIcon :icon="faXmark" />
                </button>
            </div>

            <div class="flex-1 overflow-y-auto p-4 space-y-6">
                <section>
                    <h3 class="text-xs text-cyan-300 font-mono uppercase tracking-wider mb-3">SDK</h3>
                    <div class="space-y-2">
                        <div class="flex items-center justify-between">
                            <span class="text-xs text-slate-400 font-mono">Status</span>
                            <div class="flex items-center gap-1.5">
                                <div class="w-2 h-2 rounded-full" :class="telemetry.connected ? 'bg-green-400 animate-pulse' : 'bg-red-500'" />
                                <span class="text-xs font-mono" :class="telemetry.connected ? 'text-green-400' : 'text-red-400'">
                                    {{ telemetry.connected ? 'Połączony' : 'Rozłączony' }}
                                </span>
                            </div>
                        </div>

                        <div class="flex gap-2 mt-2">
                            <button
                                class="flex-1 text-xs font-mono py-1.5 rounded border transition-all cursor-pointer"
                                :class="simulator.running.value
                                    ? 'border-red-500/50 bg-red-500/10 text-red-400 hover:bg-red-500/20'
                                    : 'border-cyan-500/50 bg-cyan-500/10 text-cyan-300 hover:bg-cyan-500/20'"
                                @click="simulator.running.value ? simulator.stop() : simulator.start()"
                            >
                                <FontAwesomeIcon :icon="simulator.running.value ? 'stop' : 'play'" class="mr-1" />
                                    {{ simulator.running.value ? 'Stop Demo' : 'Demo Mode' }}
                            </button>
                        </div>
                    </div>
                </section>

                <section>
                    <h3 class="text-xs text-cyan-300 font-mono uppercase tracking-wider mb-3">Widok</h3>
                    <div class="space-y-3">
                        <div class="flex items-center justify-between">
                            <span class="text-xs text-slate-400 font-mono">Jednostka prędkości</span>
                            <div class="flex gap-1">
                                <button
                                    v-for="unit in ['kmh', 'mph']"
                                    :key="unit"
                                    @click="settingsLocal.speedUnit = unit as any"
                                    class="text-xs font-mono px-2 py-0.5 rounded transition-all cursor-pointer"
                                    :class="settingsLocal.speedUnit === unit ? 'bg-cyan-500/20 text-cyan-300 ring-1 ring-cyan-500/70' : 'bg-slate-800 text-slate-500'"
                                >
                                    {{ unit }}
                                </button>
                            </div>
                        </div>

                        <div class="flex items-center justify-between">
                            <span class="text-xs text-slate-400 font-mono">Globalna przezroczystość</span>
                            <span class="text-xs text-cyan-300 font-mono">{{ settingsLocal.overlayGlobalOpacity }}%</span>
                        </div>

                        <input
                            type="range" min="20" max="100"
                            class="w-full accent-cyan-400"
                            v-model.number="settingsLocal.overlayGlobalOpacity"
                        />
                    </div>
                </section>

                <section class="overflow-auto">
                    <div class="flex items-center justify-between mb-3">
                        <h3 class="text-xs text-cyan-300 font-mono uppercase tracking-wider">Widgety</h3>
                        <button
                            v-if="hiddenWidgetsCount > 0"
                            class="text-[11px] font-mono px-2 py-1 rounded border border-emerald-500/40 bg-emerald-500/10 text-emerald-400 hover:bg-emerald-500/20 transition-colors cursor-pointer"
                            @click="layout.restoreHiddenWidgets()"
                        >
                            Przywróć ukryte ({{ hiddenWidgetsCount }})
                        </button>
                    </div>
                    <div class="space-y-1.5 overflow-auto max-h-70">
                        <div
                            v-for="widget in widgetsWithMeta"
                            class="flex items-center justify-between px-2 py-1.5 rounded bg-slate-800/45"
                            :key="widget.id"
                        >
                            <div class="flex items-center gap-2">
                                <FontAwesomeIcon :icon="widget.meta?.icon || 'cubes'" class="text-slate-500 text-xs w-4" />
                                <span class="text-xs text-slate-300 font-mono">{{ widget.meta?.label || widget.id }}</span>
                            </div>

                            <div class="flex items-center gap-2">
                                <button
                                    class="text-xs transition-colors cursor-pointer"
                                    :class="widget.locked ? 'text-cyan-300' : 'text-slate-600 hover:text-slate-300'"
                                    :title="widget.locked ? 'Zablokowany' : 'Odblokowany'"
                                    @click="layout.toggleLocked(widget.id)"
                                >
                                    <FontAwesomeIcon :icon="widget.locked ? faLock : faLockOpen" />
                                </button>

                                <button
                                    class="text-[11px] font-mono px-2 py-0.5 rounded border transition-colors cursor-pointer"
                                    :class="widget.visible
                                        ? 'border-slate-700 bg-slate-800 text-slate-300 hover:border-red-500/50 hover:text-red-300'
                                        : 'border-emerald-500/40 bg-emerald-500/10 text-emerald-400 hover:bg-emerald-500/20'"
                                    @click="layout.setVisible(widget.id, !widget.visible)"
                                >
                                    {{ widget.visible ? 'Ukryj' : 'Przywróć' }}
                                </button>
                            </div>
                        </div>
                    </div>
                </section>

                <section>
                    <h3 class="text-xs text-cyan-300 font-mono uppercase tracking-wider mb-3">Układ</h3>
                    <div class="flex gap-2">
                        <button
                            class="flex-1 text-xs font-mono py-1.5 rounded border transition-all cursor-pointer"
                            :class="layout.editMode ? 'border-cyan-500/50 bg-cyan-500/10 text-cyan-300' : 'border-slate-700 bg-slate-800 text-slate-400 hover:border-slate-600'"
                            @click="layout.toggleEditMode()"
                        >
                            <FontAwesomeIcon :icon="faArrowsUpDownLeftRight" class="mr-1" />
                            {{ layout.editMode ? 'Zakończ edycję' : 'Edytuj układ' }}
                        </button>

                        <button
                            class="text-xs font-mono py-1.5 px-3 rounded border border-slate-700 bg-slate-800 text-slate-400 hover:border-red-500/50 hover:text-red-400 transition-all cursor-pointer"
                            @click="confirmReset"
                        >
                            <FontAwesomeIcon :icon="faRotateLeft" />
                        </button>
                    </div>
                </section>

            </div>

            <div class="p-4 border-t border-slate-600/40">
                <button
                    class="w-full py-2 text-xs font-mono font-bold bg-cyan-500/20 text-cyan-300 hover:bg-cyan-500/30 rounded border border-cyan-500/50 transition-all cursor-pointer"
                    @click="saveSettings"
                >
                    <FontAwesomeIcon :icon="faFloppyDisk" class="mr-1.5" />
                    Zapisz ustawienia
                </button>
            </div>
        </div>
    </Transition>
</template>