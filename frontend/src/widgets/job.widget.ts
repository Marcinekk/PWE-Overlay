import JobWidget from '@components/widgets/JobWidget.vue';
import { faBriefcase } from '@fortawesome/free-solid-svg-icons';
import type { WidgetDefinition } from '@interfaces/widget';

const definition: WidgetDefinition = {
    component: JobWidget,
    meta: {
        name: 'job',
        label: 'widgets.job.name',
        icon: faBriefcase,
        defaultPosition: { x: 20, y: 20 },
        defaultSize: { w: 300, h: 180 },
        minSize: { w: 260, h: 150 },
        when: {
            paused: false,
            onJob: true,
        }
    },
};

export default definition;
