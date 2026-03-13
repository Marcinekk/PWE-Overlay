import TruckInfoWidget from '@components/widgets/TruckInfoWidget.vue';
import { faTruck } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: TruckInfoWidget,
    meta: {
        name: 'truck-info',
        label: 'widgets.truck-info.name',
        icon: faTruck,
        defaultPosition: { x: 1620, y: 160 },
        defaultSize: { w: 280, h: 100 },
        minSize: { w: 240, h: 90 },
        when: {
            paused: false,
        }
    },
};

export default definition;
