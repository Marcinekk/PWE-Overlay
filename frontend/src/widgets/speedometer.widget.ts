import SpeedometerWidget from '@components/widgets/SpeedometerWidget.vue';
import { faGaugeHigh } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: SpeedometerWidget,
    meta: {
        name: 'speedometer',
        label: 'widgets.speedometer.name',
        icon: faGaugeHigh,
        defaultPosition: { x: 20, y: 520 },
        defaultSize: { w: 200, h: 200 },
        minSize: { w: 160, h: 160 },
        when: {
            paused: false,
        }
    },
};

export default definition;
