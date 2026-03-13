import AlertWidget from '@components/widgets/AlertWidget.vue';
import { faExclamation } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: AlertWidget,
    meta: {
        name: 'alert',
        label: 'widgets.alerts.name',
        icon: faExclamation,
        defaultPosition: { x: 580, y: 580 },
        defaultSize: { w: 320, h: 140 },
        minSize: { w: 160, h: 120 },
    },
};

export default definition;
