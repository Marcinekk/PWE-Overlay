import { createRouter, createWebHistory } from 'vue-router';
import OverlayView from '@pages/OverlayView.vue';
import PauseMenuView from '@pages/PauseMenuView.vue';

const router = createRouter({
    history: createWebHistory(),
    routes: [
        { path: '/index.html', redirect: '/' },
        { path: '/', component: OverlayView },
        { path: '/pause-menu', component: PauseMenuView },
    ],
});

export default router;