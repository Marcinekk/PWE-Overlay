import TimeWidget from '@components/widgets/TimeWidget.vue';
import { faClock } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: TimeWidget,
    meta: {
        name: 'time',
        label: 'widgets.time.name',
        icon: faClock,
        defaultPosition: { x: 1620, y: 280 },
        defaultSize: { w: 280, h: 80 },
        minSize: { w: 240, h: 70 },
        when: {
            paused: false,
        }
    },
};

export default definition;
