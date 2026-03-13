#include "G27LEDManager.hpp"
#include <algorithm>
#include "../PWEOverlay.hpp"
#include <chrono>
#include <cstdio>
#include <string>

namespace PWE {
    namespace {
        bool SendLedMask(hid_device* handle, uint8_t mask) {
            if (!handle) return false;

            unsigned char report8[8] = {0x00, 0xF8, 0x12, static_cast<unsigned char>(mask & 0x1F), 0x00, 0x00, 0x00, 0x00};
            int w8 = hid_write(handle, report8, sizeof(report8));
            if (w8 >= 0) return true;

            unsigned char report7[7] = {0xF8, 0x12, static_cast<unsigned char>(mask & 0x1F), 0x00, 0x00, 0x00, 0x00};
            int w7 = hid_write(handle, report7, sizeof(report7));
            if (w7 >= 0) return true;

            unsigned char feature8[8] = {0x00, 0xF8, 0x12, static_cast<unsigned char>(mask & 0x1F), 0x00, 0x00, 0x00, 0x00};
            int f8 = hid_send_feature_report(handle, feature8, sizeof(feature8));
            return f8 >= 0;
        }

        uint64_t GetTickMs() {
            const auto now = std::chrono::steady_clock::now().time_since_epoch();
            return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
        }
    } // namespace

    bool G27LEDManager::Init() {
        if (m_handle) return true;

        if (hid_init() != 0) {
            if (g_ctx.loadAPI && g_ctx.loadAPI->logger && g_ctx.loggerHandle) g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_ERROR, "PWEOverlay: hid_init failed.");
            return false;
        }

        static constexpr unsigned short kLogitechVid = 0x046D;
        static constexpr unsigned short kCandidatePids[] = {
            0xC29B, // G27
            0xC29A, // DFGT
            0xC299  // G25
        };

        for (const unsigned short pid : kCandidatePids) {
            m_handle = hid_open(kLogitechVid, pid, nullptr);
            if (m_handle) break;
        }

        if (!m_handle && g_ctx.loadAPI && g_ctx.loadAPI->logger && g_ctx.loggerHandle) {
            g_ctx.loadAPI->logger->Log(g_ctx.loggerHandle, SPF_LOG_WARN, "PWEOverlay: failed to open Logitech wheel HID (tried PID C29B/C29A/C299).");
        }

        return m_handle != nullptr;
    }

    void G27LEDManager::SetRPM(int rpm, int rpmMax) {
        if (!m_handle || rpmMax <= 0) return;

        const uint64_t nowMs = GetTickMs();
        uint8_t ledMask = 0;

        if (rpm >= rpmMax) {
            static constexpr uint64_t kBlinkIntervalMs = 120;
            if (m_lastLimiterBlinkTickMs == 0) {
                m_lastLimiterBlinkTickMs = nowMs;
            } else if ((nowMs - m_lastLimiterBlinkTickMs) >= kBlinkIntervalMs) {
                m_limiterBlinkOn = !m_limiterBlinkOn;
                m_lastLimiterBlinkTickMs = nowMs;
            }
            ledMask = m_limiterBlinkOn ? 0x15 : 0x0A;
        } else {
            float pct = std::clamp(static_cast<float>(rpm) / rpmMax, 0.0f, 1.0f);
            int bars = static_cast<int>(pct * 5.0f + 0.5f);
            bars = std::clamp(bars, 0, 5);
            if (pct >= 0.95f) bars = 5;
            ledMask = (bars <= 0) ? 0x00 : static_cast<uint8_t>((1u << bars) - 1u);
            m_limiterBlinkOn = true;
            m_lastLimiterBlinkTickMs = nowMs;
        }

        const bool ok = SendLedMask(m_handle, ledMask);
        if (!ok && g_ctx.loadAPI && g_ctx.loadAPI->logger && g_ctx.loggerHandle) {
            g_ctx.loadAPI->logger->LogThrottled(g_ctx.loggerHandle, SPF_LOG_ERROR, "PWEOverlay.g27.led_write_failed", 2000, "PWEOverlay: failed to send G27 LED command.");
        }
    }

    void G27LEDManager::Shutdown() {
        if (m_handle) {
            SendLedMask(m_handle, 0x00);
            hid_close(m_handle);
            m_handle = nullptr;
        }
        hid_exit();
    }
} // namespace PWE
