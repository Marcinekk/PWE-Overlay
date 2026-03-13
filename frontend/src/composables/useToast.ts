import { toast } from 'vue-sonner';
import { markRaw } from 'vue';

import DefaultAlert from '@components/Alert.vue';

function useToast() {
    const show = (options: any) => toast.custom(markRaw(DefaultAlert), { componentProps: options, ...options });
    return { show };
}

export { useToast };