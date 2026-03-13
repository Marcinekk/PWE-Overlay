import { ref, shallowRef } from 'vue';
import type { Component } from 'vue';

const isOpen = ref(false);
const currentModal = shallowRef<Component | null>(null);
const modalProps = ref<Record<string, any>>({});

function useModal() {
    const open = (component: Component, props: Record<string, any> = {}) => {
        currentModal.value = component;
        modalProps.value = props;
        isOpen.value = true;
    };

    const close = () => {
        isOpen.value = false;
        setTimeout(() => {
            currentModal.value = null;
            modalProps.value = {};
        }, 300);
    };

    return { isOpen, currentModal, modalProps, open, close };
}

export { useModal };