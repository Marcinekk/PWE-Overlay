import TachometerWidget from '@components/widgets/TachometerWidget.vue';
import { faCircleNotch } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: TachometerWidget,
    meta: {
        name: 'tachometer',
        label: 'widgets.tachometer.name',
        icon: faCircleNotch,
        defaultPosition: { x: 240, y: 520 },
        defaultSize: { w: 200, h: 200 },
        minSize: { w: 160, h: 160 },
        when: {
            paused: false,
        }
    },
};

export default definition;
