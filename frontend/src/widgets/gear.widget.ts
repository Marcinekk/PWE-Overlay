import GearWidget from '@components/widgets/GearWidget.vue';
import { faGears } from '@fortawesome/free-solid-svg-icons'
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: GearWidget,
    meta: {
        name: 'gear',
        label: 'widgets.gear.name',
        icon: faGears,
        defaultPosition: { x: 460, y: 600 },
        defaultSize: { w: 100, h: 120 },
        minSize: { w: 90, h: 100 },
        when: {
            paused: false,
        }
    },
};

export default definition;
