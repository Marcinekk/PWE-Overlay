interface MiscSettings {
    isMultiplayer: boolean;
    gameMismatch: boolean;
    frameworkMismatch: boolean;
    webViewFocus: boolean;
    onAction: boolean;
}

const defaults = {
    isMultiplayer: false,
    gameMismatch: false,
    frameworkMismatch: false,
    webViewFocus: false,
    onAction: false,
}

export type { MiscSettings };
export { defaults};