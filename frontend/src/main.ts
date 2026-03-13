import { createApp } from 'vue';
import { createPinia } from 'pinia';
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome';
import router from './router';

import App from './App.vue';
import 'vue-sonner/style.css';
import 'animate.css';
import '@assets/style.css';
import 'flowbite';

import { PerfectScrollbarPlugin } from 'vue3-perfect-scrollbar';
import 'vue3-perfect-scrollbar/style.css';

const app = createApp(App);
const pinia = createPinia();

app.component('FontAwesomeIcon', FontAwesomeIcon);

app.use(pinia);
app.use(PerfectScrollbarPlugin);
app.use(router);

app.mount('#app');