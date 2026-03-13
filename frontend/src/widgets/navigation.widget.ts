import NavigationWidget from '@components/widgets/NavigationWidget.vue';
import { faLocationDot } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: NavigationWidget,
    meta: {
        name: 'navigation',
        label: 'widgets.navigation.name',
        icon: faLocationDot,
        defaultPosition: { x: 1620, y: 20 },
        defaultSize: { w: 280, h: 120 },
        minSize: { w: 220, h: 100 },
        when: {
            paused: false,
        }
    },
};

export default definition;
