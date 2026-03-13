interface PluginResponsePayload {
    requestId?: string;
    ok?: boolean;
    message?: string;
    balance?: number;
}

type PluginCommandPayload = Record<string, unknown>;
type PluginSender = ((message: Record<string, unknown>) => boolean) | null;

export type { PluginResponsePayload, PluginCommandPayload, PluginSender };