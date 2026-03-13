import NameWidget from '@components/widgets/NameWidget.vue';
import { faGaugeHigh } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: NameWidget,
    meta: {
        name: 'name',
        label: 'widgets.name.name',
        icon: faGaugeHigh,
        defaultPosition: { x: 20, y: 520 },
        defaultSize: { w: 50, h: 30 },
        minSize: { w: 50, h: 30 },
    },
};

export default definition;
