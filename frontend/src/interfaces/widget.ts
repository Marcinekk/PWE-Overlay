import type { Component } from 'vue';

type WidgetId = string;

interface WidgetPosition {
    x: number;
    y: number;
}

interface WidgetSize {
    w: number;
    h: number;
}

interface WidgetMeta {
    name: WidgetId;
    label: string;
    icon: any;
    defaultPosition: WidgetPosition;
    defaultSize: WidgetSize;
    minSize?: WidgetSize;
    maxSize?: WidgetSize;
    defaultVisible?: boolean;
    defaultLocked?: boolean;
    defaultOpacity?: number;
    when?: {
        paused?: boolean;
        focus?: boolean;
        onJob?: boolean;
        onAction?: boolean;
        connected?: boolean;
        trailerAttached?: boolean;
        engineOn?: boolean;
        moving?: boolean;
    };
}

interface WidgetConfig {
    id: WidgetId;
    position: WidgetPosition;
    size: WidgetSize;
    visible: boolean;
    locked: boolean;
    opacity: number;
}

interface WidgetDefinition {
    meta: WidgetMeta;
    component: Component;
}

type WidgetModule = {
    default?: WidgetDefinition;
};

export type { WidgetId, WidgetPosition, WidgetSize, WidgetMeta, WidgetConfig, WidgetDefinition, WidgetModule };