#include "../Internal/PWEOverlayInternal.hpp"
#include "Logitech/G27LEDManager.hpp"

#include <chrono>
#include <sstream>
#include <string>

#include "../PWEWebView.hpp"
#include "../Hooks/bank_withdraw/WithdrawHook.hpp"

namespace PWE::Internal {
    bool isAction = false;

    namespace {
        std::string JsonEscape(const char* src) {
            if (!src) return "";
            std::string out;
            for (const unsigned char c : std::string(src)) {
                switch (c) {
                    case '\\': out += "\\\\"; break;
                    case '\"': out += "\\\""; break;
                    case '\n': out += "\\n"; break;
                    case '\r': out += "\\r"; break;
                    case '\t': out += "\\t"; break;
                    default:
                        if (c >= 0x20) out += static_cast<char>(c);
                        break;
                }
            }
            return out;
        }
    }  // namespace

    std::string BuildTelemetryMessageJson() {
        if (!g_ctx.coreAPI || !g_ctx.coreAPI->telemetry || !g_ctx.telemetryHandle) {
            return R"({"type":"telemetry/state","payload":{"connected":false}})";
        }

        SPF_CommonData commonData{};
        SPF_GameState gameState{};
        SPF_TruckConstants truckConstants{};
        SPF_TruckData truckData{};
        SPF_JobConstants jobConstants{};
        SPF_JobData jobData{};
        SPF_NavigationData navData{};
        SPF_Trailer trailers[SPF_TELEMETRY_TRAILER_MAX_COUNT]{};
        uint32_t trailerCount = SPF_TELEMETRY_TRAILER_MAX_COUNT;

        g_ctx.coreAPI->telemetry->Tel_GetCommonData(g_ctx.telemetryHandle, &commonData, sizeof(commonData));
        g_ctx.coreAPI->telemetry->Tel_GetGameState(g_ctx.telemetryHandle, &gameState, sizeof(gameState));
        g_ctx.coreAPI->telemetry->Tel_GetTruckConstants(g_ctx.telemetryHandle, &truckConstants, sizeof(truckConstants));
        g_ctx.coreAPI->telemetry->Tel_GetTruckData(g_ctx.telemetryHandle, &truckData, sizeof(truckData));
        g_ctx.coreAPI->telemetry->Tel_GetJobConstants(g_ctx.telemetryHandle, &jobConstants, sizeof(jobConstants));
        g_ctx.coreAPI->telemetry->Tel_GetJobData(g_ctx.telemetryHandle, &jobData, sizeof(jobData));
        g_ctx.coreAPI->telemetry->Tel_GetNavigationData(g_ctx.telemetryHandle, &navData, sizeof(navData));
        g_ctx.coreAPI->telemetry->Tel_GetTrailers(g_ctx.telemetryHandle, trailers, sizeof(SPF_Trailer), &trailerCount);

        const SPF_Trailer* activeTrailer = (trailerCount > 0) ? &trailers[0] : nullptr;
        const bool trailerAttached = (activeTrailer && activeTrailer->data.connected);
        const double speedKmh = static_cast<double>(truckData.speed) * 3.6;
        const double speedLimitKmh = static_cast<double>(navData.navigation_speed_limit) * 3.6;
        const double cruiseKmh = static_cast<double>(truckData.cruise_control_speed) * 3.6;
        const double adblueCapacity = static_cast<double>(truckConstants.adblue_capacity);

        std::ostringstream ss;
        ss.setf(std::ios::fixed);
        ss.precision(3);
        ss << R"({"type":"telemetry/frame","payload":{)";
        ss << R"("speed":)" << speedKmh;
        ss << R"(,"speedLimit":)" << speedLimitKmh;
        ss << R"(,"cruiseControl":)" << (truckData.cruise_control_speed > 0.1f ? "true" : "false");
        ss << R"(,"cruiseControlSpeed":)" << cruiseKmh;
        ss << R"(,"rpm":)" << truckData.engine_rpm;
        ss << R"(,"rpmMax":)" << truckConstants.rpm_limit;
        ss << R"(,"gear":)" << truckData.gear;
        ss << R"(,"gearDashboard":)" << truckData.displayed_gear;
        ss << R"(,"engineOn":)" << (truckData.engine_enabled ? "true" : "false");
        ss << R"(,"engineEnabled":)" << (truckData.engine_enabled ? "true" : "false");
        ss << R"(,"fuel":)" << truckData.fuel_amount;
        ss << R"(,"fuelMax":)" << truckConstants.fuel_capacity;
        ss << R"(,"fuelAvgConsumption":)" << truckData.fuel_average_consumption;
        ss << R"(,"fuelRange":)" << truckData.fuel_range;
        ss << R"(,"truckWear":)"
           << ((truckData.wear_engine + truckData.wear_transmission + truckData.wear_cabin + truckData.wear_chassis +
                truckData.wear_wheels) /
               5.0f);
        ss << R"(,"engineWear":)" << truckData.wear_engine;
        ss << R"(,"transmissionWear":)" << truckData.wear_transmission;
        ss << R"(,"cabinWear":)" << truckData.wear_cabin;
        ss << R"(,"chassisWear":)" << truckData.wear_chassis;
        ss << R"(,"wheelsWear":)" << truckData.wear_wheels;
        ss << R"(,"trailerAttached":)" << (trailerAttached ? "true" : "false");
        ss << R"(,"trailerWear":)" << (trailerAttached ? activeTrailer->data.wear_chassis : 0.0f);
        ss << R"(,"trailerMass":)" << (trailerAttached ? jobConstants.cargo_mass : 0.0f);
        ss << R"(,"trailerName":")" << JsonEscape(trailerAttached ? activeTrailer->constants.name : "") << "\"";
        ss << R"(,"trailerCargoMass":)" << jobConstants.cargo_mass;
        ss << R"(,"jobIncome":)" << jobConstants.income;
        ss << R"(,"jobDeadline":)" << jobConstants.delivery_time;
        ss << R"(,"jobRemainingTime":)" << (static_cast<uint32_t>(jobData.remaining_delivery_minutes) * 60);
        ss << R"(,"jobSourceCity":")" << JsonEscape(jobConstants.source_city) << "\"";
        ss << R"(,"jobSourceCompany":")" << JsonEscape(jobConstants.source_company) << "\"";
        ss << R"(,"jobDestinationCity":")" << JsonEscape(jobConstants.destination_city) << "\"";
        ss << R"(,"jobDestinationCompany":")" << JsonEscape(jobConstants.destination_company) << "\"";
        ss << R"(,"jobDistance":)" << jobConstants.planned_distance_km;
        ss << R"(,"jobDistanceRemaining":)" << navData.navigation_distance;
        ss << R"(,"jobCargoName":")" << JsonEscape(jobConstants.cargo_name) << "\"";
        ss << R"(,"onJob":)" << (jobData.on_job ? "true" : "false");
        ss << R"(,"navDistance":)" << navData.navigation_distance;
        ss << R"(,"navTime":)" << navData.navigation_time;
        ss << R"(,"navSpeedLimit":)" << speedLimitKmh;
        ss << R"(,"blinkerLeft":)" << (truckData.light_lblinker ? "true" : "false");
        ss << R"(,"blinkerRight":)" << (truckData.light_rblinker ? "true" : "false");
        ss << R"(,"hazardBlinker":)" << (truckData.hazard_warning ? "true" : "false");
        ss << R"(,"lightsParking":)" << (truckData.light_parking ? "true" : "false");
        ss << R"(,"lightsBeamLow":)" << (truckData.light_low_beam ? "true" : "false");
        ss << R"(,"lightsBeamHigh":)" << (truckData.light_high_beam ? "true" : "false");
        ss << R"(,"lightsAux":)" << ((truckData.light_aux_front > 0 || truckData.light_aux_roof > 0) ? "true" : "false");
        ss << R"(,"lightsBeacon":)" << (truckData.light_beacon ? "true" : "false");
        ss << R"(,"lightsBrake":)" << (truckData.light_brake ? "true" : "false");
        ss << R"(,"lightsReverse":)" << (truckData.light_reverse ? "true" : "false");
        ss << R"(,"airPressureWarning":)" << (truckData.air_pressure_warning ? "true" : "false");
        ss << R"(,"airPressureEmergency":)" << (truckData.air_pressure_emergency ? "true" : "false");
        ss << R"(,"fuelWarning":)" << (truckData.fuel_warning ? "true" : "false");
        ss << R"(,"adblueWarning":)" << (truckData.adblue_warning ? "true" : "false");
        ss << R"(,"oilPressureWarning":)" << (truckData.oil_pressure_warning ? "true" : "false");
        ss << R"(,"waterTemperatureWarning":)" << (truckData.water_temperature_warning ? "true" : "false");
        ss << R"(,"batteryVoltageWarning":)" << (truckData.battery_voltage_warning ? "true" : "false");
        ss << R"(,"parkBrake":)" << (truckData.parking_brake ? "true" : "false");
        ss << R"(,"motorBrake":)" << (truckData.motor_brake ? "true" : "false");
        ss << R"(,"electricEnabled":)" << (truckData.electric_enabled ? "true" : "false");
        ss << R"(,"airPressure":)" << truckData.air_pressure;
        ss << R"(,"oilPressure":)" << truckData.oil_pressure;
        ss << R"(,"oilTemperature":)" << truckData.oil_temperature;
        ss << R"(,"waterTemperature":)" << truckData.water_temperature;
        ss << R"(,"batteryVoltage":)" << truckData.battery_voltage;
        ss << R"(,"adblue":)" << truckData.adblue_amount;
        ss << R"(,"adblueMax":)" << (adblueCapacity > 0.0 ? adblueCapacity : 60.0);
        ss << R"(,"gameTime":)" << commonData.game_time;
        ss << R"(,"gamePaused":)" << (gameState.paused ? "true" : "false");
        ss << R"(,"webViewFocus":)" << (g_ctx.webViewFocus ? "true" : "false");
        ss << R"(,"truckMake":")" << JsonEscape(truckConstants.brand) << "\"";
        ss << R"(,"truckModel":")" << JsonEscape(truckConstants.name) << "\"";
        ss << R"(,"truckLicensePlate":")" << JsonEscape(truckConstants.license_plate) << "\"";
        ss << R"(,"truckLicensePlateCountry":")" << JsonEscape(truckConstants.license_plate_country_id) << "\"";
        ss << R"(,"odometer":)" << truckData.odometer;
        ss << R"(,"accelerationX":)" << truckData.local_linear_acceleration.x;
        ss << R"(,"accelerationY":)" << truckData.local_linear_acceleration.y;
        ss << R"(,"accelerationZ":)" << truckData.local_linear_acceleration.z;
        ss << R"(,"coordinateX":)" << truckData.world_placement.position.x;
        ss << R"(,"coordinateY":)" << truckData.world_placement.position.y;
        ss << R"(,"coordinateZ":)" << truckData.world_placement.position.z;
        ss << R"(,"rotationX":)" << truckData.world_placement.orientation.heading;
        ss << R"(,"rotationY":)" << truckData.world_placement.orientation.pitch;
        ss << R"(,"rotationZ":)" << truckData.world_placement.orientation.roll;
        ss << R"(,"money":)" << PWE::Hooks::GetMoney();
        ss << R"(,"onAction":)" << (isAction ? "true" : "false");
        ss << "}}";
        return ss.str();
    }

    void UpdateG27LEDs() {
        if (!g_ctx.coreAPI || !g_ctx.coreAPI->telemetry || !g_ctx.telemetryHandle) return;

        SPF_TruckConstants truckConstants{};
        SPF_TruckData truckData{};
        g_ctx.coreAPI->telemetry->Tel_GetTruckConstants(g_ctx.telemetryHandle, &truckConstants, sizeof(truckConstants));
        g_ctx.coreAPI->telemetry->Tel_GetTruckData(g_ctx.telemetryHandle, &truckData, sizeof(truckData));
        PWE::g27LED.SetRPM(static_cast<int>(truckData.engine_rpm), static_cast<int>(truckConstants.rpm_limit));
    }

    void PushTelemetryToWebView() {
        if (!g_ctx.showWebView) return;
        const auto now =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        if (now - static_cast<int64_t>(g_ctx.lastTelemetryPushTickMs) < 50) return;
        g_ctx.lastTelemetryPushTickMs = static_cast<uint64_t>(now);
        const std::string json = BuildTelemetryMessageJson();
        PWE::PostWebViewOverlayMessageJson(json.c_str());
    }
}  // namespace PWE::Internal