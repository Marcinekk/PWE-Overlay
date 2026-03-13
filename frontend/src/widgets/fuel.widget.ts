import FuelWidget from '@components/widgets/FuelWidget.vue';
import { faGasPump } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: FuelWidget,
    meta: {
        name: 'fuel',
        label: 'widgets.fuel.name',
        icon: faGasPump,
        defaultPosition: { x: 580, y: 580 },
        defaultSize: { w: 180, h: 140 },
        minSize: { w: 160, h: 120 },
        when: {
            paused: false,
        }
    },
};

export default definition;
