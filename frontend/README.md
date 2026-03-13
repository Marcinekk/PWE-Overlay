# Vue 3 + TypeScript + Vite

This template should help get you started developing with Vue 3 and TypeScript in Vite. The template uses Vue 3 `<script setup>` SFCs, check out the [script setup docs](https://v3.vuejs.org/api/sfc-script-setup.html#sfc-script-setup) to learn more.

Learn more about the recommended Project Setup and IDE Support in the [Vue Docs TypeScript Guide](https://vuejs.org/guide/typescript/overview.html#project-setup).

## i18n (vue-i18n)

Tłumaczenia są ładowane w runtime z plików JSON, żeby dało się je edytować po buildzie bez przebudowy frontendu.

- Źródła: `frontend/public/locales/en.json`, `frontend/public/locales/pl.json`
- Po buildzie: `frontend/dist/locales/*.json` (możesz podmienić/edytować pliki na serwerze)
- Kod ładowania: `frontend/src/i18n.ts`

Dodanie nowego języka wymaga:
1) dodania pliku `frontend/public/locales/<lang>.json`
2) dopisania języka do `SUPPORTED_LOCALES` w `frontend/src/i18n.ts` (a potem build)
