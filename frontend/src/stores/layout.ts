import { computed, ref } from 'vue';
import { defineStore } from 'pinia';
import type { WidgetConfig, WidgetId } from '@interfaces/widget';
import { getDefaultWidgets, getWidgetMeta } from '@widgets/index';

const STORAGE_KEY = 'pwe-widget-layout';

function cloneWidgetConfig(widget: WidgetConfig): WidgetConfig {
    return {
        ...widget,
        position: { ...widget.position },
        size: { ...widget.size },
    };
}

function clamp(value: number, min: number, max: number): number {
    return Math.min(Math.max(value, min), max);
}

function normalizeWidgetConfig(candidate: Partial<WidgetConfig>, fallback: WidgetConfig): WidgetConfig {
    const meta = getWidgetMeta(fallback.id);
    const minSize = meta?.minSize ?? { w: 80, h: 80 };
    const maxSize = meta?.maxSize ?? { w: Number.POSITIVE_INFINITY, h: Number.POSITIVE_INFINITY };

    const size = candidate.size ?? fallback.size;
    const normalizedSize = {
        w: clamp(size.w, minSize.w, maxSize.w),
        h: clamp(size.h, minSize.h, maxSize.h),
    };

    const position = candidate.position ?? fallback.position;
    return {
        id: fallback.id,
        position: {
            x: Math.max(0, position.x),
            y: Math.max(0, position.y),
        },
        size: normalizedSize,
        visible: candidate.visible ?? fallback.visible,
        locked: candidate.locked ?? fallback.locked,
        opacity: clamp(candidate.opacity ?? fallback.opacity, 20, 100),
    };
}

function buildWidgetFromMeta(id: WidgetId): WidgetConfig | undefined {
    const meta = getWidgetMeta(id);
    if (!meta) return undefined;
    return {
        id: meta.name,
        position: { ...meta.defaultPosition },
        size: { ...meta.defaultSize },
        visible: meta.defaultVisible ?? true,
        locked: meta.defaultLocked ?? false,
        opacity: meta.defaultOpacity ?? 90,
    };
}

const useLayoutStore = defineStore('layout', () => {
    const widgets = ref<WidgetConfig[]>(loadLayout());
    const editMode = ref(false);
    const editMenu = ref(false);

    function loadLayout(): WidgetConfig[] {
        const defaults = getDefaultWidgets();
        try {
            const saved = localStorage.getItem(STORAGE_KEY);
            if (saved) {
                const parsed = JSON.parse(saved) as Partial<WidgetConfig>[];
                return defaults.map((fallback: WidgetConfig) => {
                    const match = parsed.find((item) => item.id === fallback.id);
                    return normalizeWidgetConfig(match ?? {}, fallback);
                });
            }
        } catch {}
        return defaults.map(cloneWidgetConfig);
    }

    function saveLayout() {
        localStorage.setItem(STORAGE_KEY, JSON.stringify(widgets.value));
    }

    function getWidget(id: WidgetId): WidgetConfig | undefined {
        return widgets.value.find((w: WidgetConfig) => w.id === id);
    }

    function ensureWidget(id: WidgetId): WidgetConfig | undefined {
        const existing = getWidget(id);
        if (existing) return existing;

        const created = buildWidgetFromMeta(id);
        if (!created) return undefined;
        widgets.value.push(created);
        saveLayout();
        return created;
    }

    function updatePosition(id: WidgetId, x: number, y: number) {
        const w = getWidget(id);
        if (w) {
            w.position = { x, y };
            saveLayout();
        }
    }

    function updateSize(id: WidgetId, width: number, height: number) {
        const widget = getWidget(id);
        const meta = getWidgetMeta(id);
        if (!widget) return;

        const minSize = meta?.minSize ?? { w: 80, h: 80 };
        const maxSize = meta?.maxSize ?? { w: Number.POSITIVE_INFINITY, h: Number.POSITIVE_INFINITY };
        widget.size = {
            w: clamp(width, minSize.w, maxSize.w),
            h: clamp(height, minSize.h, maxSize.h),
        };
        saveLayout();
    }

    function updateOpacity(id: WidgetId, opacity: number) {
        const w = getWidget(id);
        if (w) {
            w.opacity = opacity;
            saveLayout();
        }
    }

    function toggleVisible(id: WidgetId) {
        const w = ensureWidget(id);
        if (w) {
            w.visible = !w.visible;
            saveLayout();
        }
    }

    function setVisible(id: WidgetId, visible: boolean) {
        const w = ensureWidget(id);
        if (w) {
            w.visible = visible;
            saveLayout();
        }
    }

    function restoreHiddenWidgets() {
        let changed = false;
        for (const widget of widgets.value) {
            if (!widget.visible) {
                widget.visible = true;
                changed = true;
            }
        }
        if (changed) saveLayout();
    }

    function toggleLocked(id: WidgetId) {
        const w = ensureWidget(id);
        if (w) {
            w.locked = !w.locked;
            saveLayout();
        }
    }

    function resetLayout() {
        widgets.value = getDefaultWidgets().map(cloneWidgetConfig);
        saveLayout();
    }

    function toggleEditMode() {
        editMode.value = !editMode.value;
    }

    function toggleSettingsMenu() {
        editMenu.value = !editMenu.value;
    }

    const visibleWidgets = computed(() => widgets.value.filter((w: WidgetConfig) => w.visible));

    return {
        editMenu,
        widgets,
        visibleWidgets,
        editMode,
        getWidget,
        updatePosition,
        updateSize,
        updateOpacity,
        toggleVisible,
        setVisible,
        restoreHiddenWidgets,
        toggleLocked,
        resetLayout,
        toggleEditMode,
        saveLayout,
        toggleSettingsMenu,
    };
});

export { useLayoutStore };