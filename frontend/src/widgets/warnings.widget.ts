import WarningsWidget from '@components/widgets/WarningsWidget.vue';
import { faTriangleExclamation } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: WarningsWidget,
    meta: {
        name: 'warnings',
        label: 'widgets.warnings.name',
        icon: faTriangleExclamation,
        defaultPosition: { x: 20, y: 220 },
        defaultSize: { w: 400, h: 76 },
        minSize: { w: 80, h: 76 },
        when: {
            paused: false,
        }
    },
};

export default definition;
