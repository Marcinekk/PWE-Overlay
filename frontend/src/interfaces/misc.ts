interface MiscSettings {
    isMultiplayer: boolean;
    gameMismatch: boolean;
    frameworkMismatch: boolean;
    webViewFocus: boolean;
    onAction: boolean;
    isManualTransactionApprove: boolean;
    profileName: string;
    gameName: string;
}

const defaults = {
    isMultiplayer: false,
    gameMismatch: false,
    frameworkMismatch: false,
    webViewFocus: false,
    onAction: false,
    isManualTransactionApprove: false,
    profileName: '',
    gameName: '',
}

export type { MiscSettings };
export { defaults};