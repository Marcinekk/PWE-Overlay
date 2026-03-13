import LightsWidget from '@components/widgets/LightsWidget.vue';
import { faLightbulb } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: LightsWidget,
    meta: {
        name: 'lights',
        label: 'widgets.lights.name',
        icon: faLightbulb,
        defaultPosition: { x: 800, y: 620 },
        defaultSize: { w: 200, h: 100 },
        minSize: { w: 170, h: 90 },
        when: {
            paused: false,
        }
    },
};

export default definition;
