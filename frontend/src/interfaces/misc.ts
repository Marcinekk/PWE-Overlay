interface MiscSettings {
    isMultiplayer: boolean;
    gameMismatch: boolean;
    frameworkMismatch: boolean;
    webViewFocus: boolean;
    onAction: boolean;
    isManualTransactionApprove: boolean;
}

const defaults = {
    isMultiplayer: false,
    gameMismatch: false,
    frameworkMismatch: false,
    webViewFocus: false,
    onAction: false,
    isManualTransactionApprove: false,
}

export type { MiscSettings };
export { defaults};