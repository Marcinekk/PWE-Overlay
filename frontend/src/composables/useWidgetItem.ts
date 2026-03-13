import { computed } from 'vue';
import type { WidgetId } from '@interfaces/widget';
import { useLayoutStore } from '@stores/layout';
import { getWidgetMeta } from '@widgets/index';

function useWidgetItem(id: WidgetId) {
    const layout = useLayoutStore();
    const widget = computed(() => layout.getWidget(id));
    const widgetMeta = computed(() => getWidgetMeta(id));
    const editMode = computed(() => layout.editMode);

    const widgetStyle = computed(() => {
        const current = widget.value;
        if (!current) return {};
        return {
            left: `${current.position.x}px`,
            top: `${current.position.y}px`,
            width: `${current.size.w}px`,
            height: `${current.size.h}px`,
            opacity: current.opacity / 100,
        };
    });

    function onOpacityChange(event: Event) {
        layout.updateOpacity(id, Number((event.target as HTMLInputElement).value));
    }

    return { editMode, layout, onOpacityChange, widget, widgetMeta, widgetStyle };
}

export { useWidgetItem };