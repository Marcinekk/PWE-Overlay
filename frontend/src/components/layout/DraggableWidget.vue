<script setup lang="ts">
    import { computed } from 'vue';
    import { useDraggable } from '@composables/useDraggable';
    import { useResizable } from '@composables/useResizable';
    import { useWidgetItem } from '@composables/useWidgetItem';
    import { useTelemetryStore } from '@stores/telemetry';
    import { useLayoutStore } from '@stores/layout';
    import type { WidgetId } from '@interfaces/widget';

    import { faEyeSlash, faSun, faLock, faLockOpen } from '@fortawesome/free-solid-svg-icons';

    const lay = useLayoutStore();

    const props = defineProps<{ id: WidgetId }>();
    const { widget, widgetMeta, editMode, widgetStyle, onOpacityChange, layout } = useWidgetItem(props.id);
    const telemetry = useTelemetryStore();

    const { isDragging, onMouseDown } = useDraggable(props.id);
    const { isResizing, onResizeMouseDown } = useResizable(props.id);

    const visibleByRuntimeRules = computed(() => {
        if(lay.editMode) return true;

        const meta = widgetMeta.value;
        if (!meta?.when) return true;

        const moving = telemetry.data.speed > 1;
        if (typeof meta.when.paused === 'boolean' && telemetry.data.gamePaused !== meta.when.paused) return false;
        if (typeof meta.when.focus === 'boolean' && telemetry.data.webViewFocus !== meta.when.focus) return false;
        if (typeof meta.when.onJob === 'boolean' && telemetry.data.onJob !== meta.when.onJob) return false;
        if (typeof meta.when.connected === 'boolean' && telemetry.connected !== meta.when.connected) return false;
        if (typeof meta.when.trailerAttached === 'boolean' && telemetry.data.trailerAttached !== meta.when.trailerAttached) return false;
        if (typeof meta.when.engineOn === 'boolean' && telemetry.data.engineOn !== meta.when.engineOn) return false;
        if (typeof meta.when.moving === 'boolean' && moving !== meta.when.moving) return false;
        if (typeof meta.when.onAction === 'boolean' && telemetry.data.onAction !== meta.when.onAction) return false;
        return true;
    });

    const shouldRender = computed(() => {
        return Boolean(widget.value && widget.value.visible && visibleByRuntimeRules.value);
    });
</script>

<template>
    <Transition
        enter-active-class="animate__animated animate__fadeInUp animate__faster"
        leave-active-class="animate__animated animate__fadeOutDown animate__faster"
    >
        <div
            v-if="shouldRender && widget"
            class="draggable-widget absolute select-none"
            :class="{
                'cursor-move': editMode && !widget.locked,
                'ring-1 ring-cyan-500/60': editMode,
                'ring-1 ring-cyan-500/30': editMode && widget.locked,
                'transition-shadow': true,
            }"
            :style="{
                ...widgetStyle,
                zIndex: isDragging || isResizing ? 1000 : 10,
            }"
            @mousedown="onMouseDown"
        >
            <div
                v-if="editMode"
                class="absolute -top-7 left-0 flex items-center gap-1 z-50 bg-slate-950/90 border border-slate-600/40 rounded-t px-2 py-0.5"
                data-no-drag
            >
                <span class="text-xs text-slate-300 font-mono mr-1">{{ widgetMeta?.label || widget.id }}</span>

                <button
                    class="text-slate-400 hover:text-cyan-300 text-xs px-1 transition-colors"
                    :title="widget.locked ? 'Odblokuj' : 'Zablokuj'"
                    @click="layout.toggleLocked(widget.id)"
                >
                    <FontAwesomeIcon :icon="widget.locked ? faLock : faLockOpen" />
                </button>

                <button
                    class="text-slate-400 hover:text-red-400 text-xs px-1 transition-colors"
                    title="Ukryj"
                    @click="layout.toggleVisible(widget.id)"
                >
                    <FontAwesomeIcon :icon="faEyeSlash" />
                </button>

                <div class="flex items-center gap-1 ml-1" data-no-drag>
                    <FontAwesomeIcon :icon="faSun" class="text-slate-500 text-xs" />
                    <input
                        type="range"
                        min="20"
                        max="100"
                        class="w-16 h-1 accent-cyan-400"
                        data-no-drag
                        :value="widget.opacity"
                        @input="onOpacityChange"
                    />
                </div>
            </div>

            <div class="w-full h-full">
                <slot />
            </div>

            <div
                v-if="editMode && !widget.locked"
                class="absolute inset-0 border border-dashed border-cyan-500/40 pointer-events-none rounded"
            />

            <button
                v-if="editMode && !widget.locked"
                class="absolute bottom-0 right-0 w-4 h-4 cursor-nwse-resize bg-cyan-500/40 hover:bg-cyan-400/55 rounded-tl"
                title="Zmień rozmiar"
                data-no-drag
                @mousedown="onResizeMouseDown"
            />
        </div>
    </Transition>
</template>