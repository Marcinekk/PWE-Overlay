import { onMounted, onUnmounted, ref } from 'vue';
import type { WidgetId } from '@interfaces/widget';
import { useLayoutStore } from '@stores/layout';

function useResizable(id: WidgetId) {
    const layout = useLayoutStore();
    const isResizing = ref(false);
    const startMouse = ref({ x: 0, y: 0 });
    const startSize = ref({ w: 0, h: 0 });

    function onResizeMouseDown(event: MouseEvent) {
        const widget = layout.getWidget(id);
        if (!widget || widget.locked || !layout.editMode) return;

        isResizing.value = true;
        startMouse.value = { x: event.clientX, y: event.clientY };
        startSize.value = { ...widget.size };
        event.preventDefault();
        event.stopPropagation();
    }

    function onMouseMove(event: MouseEvent) {
        if (!isResizing.value) return;

        const dx = event.clientX - startMouse.value.x;
        const dy = event.clientY - startMouse.value.y;
        const width = Math.max(80, startSize.value.w + dx);
        const height = Math.max(80, startSize.value.h + dy);

        layout.updateSize(id, width, height);
    }

    function onMouseUp() {
        isResizing.value = false;
    }

    onMounted(() => {
        window.addEventListener('mousemove', onMouseMove);
        window.addEventListener('mouseup', onMouseUp);
    });

    onUnmounted(() => {
        window.removeEventListener('mousemove', onMouseMove);
        window.removeEventListener('mouseup', onMouseUp);
    });

    return { isResizing, onResizeMouseDown };
}

export { useResizable };