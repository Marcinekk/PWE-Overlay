import TachographWidget from '@components/widgets/TachographWidget.vue';
import { faStopwatch } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: TachographWidget,
    meta: {
        name: 'tachograph',
        label: 'widgets.tachograph.name',
        icon: faStopwatch,
        defaultPosition: { x: 1240, y: 270 },
        defaultSize: { w: 360, h: 220 },
        minSize: { w: 330, h: 210 },
        when: {
            onAction: true,
            paused: false,
        }
    },
};

export default definition;
