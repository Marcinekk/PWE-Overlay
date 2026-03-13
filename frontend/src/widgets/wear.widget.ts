import WearWidget from '@components/widgets/WearWidget.vue';
import { faWrench } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: WearWidget,
    meta: {
        name: 'wear',
        label: 'widgets.wear.name',
        icon: faWrench,
        defaultPosition: { x: 340, y: 20 },
        defaultSize: { w: 280, h: 160 },
        minSize: { w: 240, h: 130 },
        when: {
            paused: false,
        }
    },
};

export default definition;
