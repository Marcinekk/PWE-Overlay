#pragma once
#include "../External/hidapi/include/hidapi.h"
#include <cstdint>

namespace PWE {
    class G27LEDManager {
        public:
            G27LEDManager() = default;
            ~G27LEDManager() { Shutdown(); }

            bool Init();
            void SetRPM(int rpm, int rpmMax);
            void Shutdown();

        private:
            hid_device* m_handle = nullptr;
            bool m_limiterBlinkOn = true;
            uint64_t m_lastLimiterBlinkTickMs = 0;
    };
    extern G27LEDManager g27LED;
} // namespace PWE