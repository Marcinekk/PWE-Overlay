import { createI18n } from 'vue-i18n';

export const DEFAULT_LOCALE = 'en';
export const SUPPORTED_LOCALES = ['en', 'pl'] as const;
export type SupportedLocale = (typeof SUPPORTED_LOCALES)[number];

export const LOCALE_STORAGE_KEY = 'locale';

const loadedLocales = new Set<string>();

function normalizeLocale(value: string | null | undefined): SupportedLocale {
    const raw = (value ?? '').trim().toLowerCase();
    const base = raw.split(/[-_]/)[0] ?? '';
    return (SUPPORTED_LOCALES as readonly string[]).includes(base)
        ? (base as SupportedLocale)
        : DEFAULT_LOCALE;
}

async function fetchLocaleJson(locale: SupportedLocale): Promise<Record<string, unknown>> {
    const url = `${import.meta.env.BASE_URL}locales/${locale}.json`;
    const res = await fetch(url, { cache: 'no-cache' });
    if (!res.ok) {
        throw new Error(`Failed to load locale "${locale}" (${res.status} ${res.statusText})`);
    }
    return (await res.json()) as Record<string, unknown>;
}

async function ensureLocaleLoaded(locale: SupportedLocale): Promise<void> {
    if (loadedLocales.has(locale)) return;

    const messages = await fetchLocaleJson(locale);
    i18n.global.setLocaleMessage(locale, messages);
    loadedLocales.add(locale);
}

export const i18n = createI18n({
    legacy: false,
    globalInjection: true,
    locale: DEFAULT_LOCALE,
    fallbackLocale: DEFAULT_LOCALE,
    messages: {
        en: {},
        pl: {},
    },
});

export function getSavedLocale(): string | null {
    return localStorage.getItem(LOCALE_STORAGE_KEY);
}

export function hasLocalePreference(): boolean {
    return getSavedLocale() !== null;
}

export async function setLocale(
    localeInput: string,
    options: { persist?: boolean } = {},
): Promise<SupportedLocale> {
    const locale = normalizeLocale(localeInput);
    const persist = options.persist ?? true;

    try {
        await ensureLocaleLoaded(locale);
    } catch {
        await ensureLocaleLoaded(DEFAULT_LOCALE).catch(() => undefined);
        i18n.global.locale.value = DEFAULT_LOCALE;
        if (persist) localStorage.setItem(LOCALE_STORAGE_KEY, DEFAULT_LOCALE);
        document.documentElement.setAttribute('lang', DEFAULT_LOCALE);
        return DEFAULT_LOCALE;
    }

    i18n.global.locale.value = locale;
    if (persist) localStorage.setItem(LOCALE_STORAGE_KEY, locale);
    document.documentElement.setAttribute('lang', locale);

    return locale;
}

export async function initI18n(): Promise<SupportedLocale> {
    const saved = getSavedLocale();
    if (saved) return setLocale(saved, { persist: true });

    const detected = navigator.language ?? DEFAULT_LOCALE;
    return setLocale(detected, { persist: false });
}
