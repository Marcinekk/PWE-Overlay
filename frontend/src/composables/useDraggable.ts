import { ref, onMounted, onUnmounted } from 'vue';
import type { WidgetId } from '@interfaces/widget';
import { useLayoutStore } from '@stores/layout';

function useDraggable(id: WidgetId) {
    const layout = useLayoutStore();
    const isDragging = ref(false);
    const startMouse = ref({ x: 0, y: 0 });
    const startPos = ref({ x: 0, y: 0 });

    function onMouseDown(e: MouseEvent) {
        const widget = layout.getWidget(id);
        if (!widget || widget.locked || !layout.editMode) return;
        if ((e.target as HTMLElement).closest('[data-no-drag]')) return;

        isDragging.value = true;
        startMouse.value = { x: e.clientX, y: e.clientY };
        startPos.value = { ...widget.position };

        e.preventDefault();
    }

    function onMouseMove(e: MouseEvent) {
        if (!isDragging.value) return;
        const dx = e.clientX - startMouse.value.x;
        const dy = e.clientY - startMouse.value.y;

        const newX = Math.max(0, startPos.value.x + dx);
        const newY = Math.max(0, startPos.value.y + dy);

        layout.updatePosition(id, newX, newY);
    }

    function onMouseUp() {
        isDragging.value = false;
    }

    onMounted(() => {
        window.addEventListener('mousemove', onMouseMove);
        window.addEventListener('mouseup', onMouseUp);
    });

    onUnmounted(() => {
        window.removeEventListener('mousemove', onMouseMove);
        window.removeEventListener('mouseup', onMouseUp);
    });

    return { isDragging, onMouseDown };
}

export { useDraggable };