# Vue 3 + TypeScript + Vite

This template should help get you started developing with Vue 3 and TypeScript in Vite. The template uses Vue 3 `<script setup>` SFCs, check out the [script setup docs](https://v3.vuejs.org/api/sfc-script-setup.html#sfc-script-setup) to learn more.

Learn more about the recommended Project Setup and IDE Support in the [Vue Docs TypeScript Guide](https://vuejs.org/guide/typescript/overview.html#project-setup).

## i18n (vue-i18n)

Translations are loaded at runtime from JSON files so that they can be edited after build without rebuilding the frontend.

- Sources: `frontend/public/locales/en.json`, `frontend/public/locales/pl.json`
- After build: `frontend/dist/locales/*.json` (you can replace/edit files)
- Loading code: `frontend/src/i18n.ts`

Adding a new language requires:

1. adding a file `frontend/public/locales/<lang>.json`
2. adding language to `SUPPORTED_LOCALES` in `frontend/src/i18n.ts` (then build)
