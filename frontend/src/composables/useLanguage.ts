import { i18n } from '../i18n';

function Locale(str: string) {
    return i18n.global.t(str) || str;
}

export { Locale };