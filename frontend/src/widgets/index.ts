import type { WidgetConfig, WidgetDefinition, WidgetId, WidgetMeta, WidgetModule } from '@interfaces/widget';

const modules = import.meta.glob('./*.widget.ts', { eager: true }) as Record<string, WidgetModule>;

const definitions = Object.values(modules)
    .map((mod) => mod.default)
    .filter((def): def is WidgetDefinition => Boolean(def));

const dedupedDefinitions = Array.from(
    new Map(definitions.map((def) => [def.meta.name, def])).values(),
);

const widgetDefinitions = dedupedDefinitions.sort((a, b) => a.meta.label.localeCompare(b.meta.label, 'pl'));

const widgetRegistry = new Map<WidgetId, WidgetDefinition>(
    widgetDefinitions.map((def) => [def.meta.name, def]),
);

function getWidgetDefinition(id: WidgetId): WidgetDefinition | undefined {
    return widgetRegistry.get(id);
}

function getWidgetMeta(id: WidgetId): WidgetMeta | undefined {
    return getWidgetDefinition(id)?.meta;
}

function getDefaultWidgets(): WidgetConfig[] {
    return widgetDefinitions.map((definition) => {
        const meta = definition.meta;
        return {
            id: meta.name,
            position: { ...meta.defaultPosition },
            size: { ...meta.defaultSize },
            visible: meta.defaultVisible ?? true,
            locked: meta.defaultLocked ?? false,
            opacity: meta.defaultOpacity ?? 90,
        };
    });
}

export { getDefaultWidgets, getWidgetDefinition, getWidgetMeta, widgetDefinitions, widgetRegistry };