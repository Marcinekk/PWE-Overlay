#include "job_event.hpp"
#include "../../PWEOverlay.hpp"
#include "../../Storage/Jobs/Jobs.hpp"
#include <SPF_API/SPF_TelemetryData.h>
#include <string>

namespace PWE::Events::Job {
    bool jobDataRegistered = false;
    bool specialEventsRegistered = false;

    namespace {
        SPF_Telemetry_Callback_Handle* g_jobDataEvent = nullptr;
        SPF_Telemetry_Callback_Handle* g_specialEventsEvent = nullptr;
        
        bool g_lastOnJob = false;
        
        void OnJobDataUpdate(const SPF_JobData* jobData, void* user_data) {
            if (!jobData) return;
            
            if (jobData->on_job && !g_lastOnJob) {
                //Something soon..
            }
            
            g_lastOnJob = jobData->on_job;
        }
        
        void OnSpecialEventsUpdate(const SPF_SpecialEvents* events, void* user_data) {
            if (!events) return;
            if (events->job_delivered || events->job_cancelled) {
                SPF_JobConstants jobConst{};
                SPF_GameplayEvents gameplayEvents{};
                
                g_ctx.coreAPI->telemetry->Tel_GetJobConstants(g_ctx.telemetryHandle, &jobConst, sizeof(SPF_JobConstants));
                g_ctx.coreAPI->telemetry->Tel_GetGameplayEvents(g_ctx.telemetryHandle, &gameplayEvents, sizeof(SPF_GameplayEvents));
                
                if (events->job_delivered) {
                    PWE::Storage::Jobs::Insert(
                        jobConst.cargo_name,
                        jobConst.source_city,
                        jobConst.source_company,
                        jobConst.destination_city,
                        jobConst.destination_company,
                        gameplayEvents.job_delivered.revenue,
                        gameplayEvents.job_delivered.earned_xp,
                        gameplayEvents.job_delivered.distance_km,
                        "delivered",
                        jobConst.cargo_unit_mass,
                        jobConst.is_special_job
                    );
                } else if (events->job_cancelled) {
                    PWE::Storage::Jobs::Insert(
                        jobConst.cargo_name,
                        jobConst.source_city,
                        jobConst.source_company,
                        jobConst.destination_city,
                        jobConst.destination_company,
                        -gameplayEvents.job_cancelled.penalty,
                        0,
                        0.0f,
                        "cancelled",
                        jobConst.cargo_unit_mass,
                        jobConst.is_special_job
                    );
                }
            }
        }
    }

    void Register() {
        if (!g_ctx.telemetryHandle || !g_ctx.coreAPI || !g_ctx.coreAPI->telemetry) return;
        
        if (!g_jobDataEvent && !jobDataRegistered) {
            g_jobDataEvent = g_ctx.coreAPI->telemetry->Tel_RegisterForJobData(g_ctx.telemetryHandle, OnJobDataUpdate, nullptr);
            jobDataRegistered = true;
        }
        
        if (!g_specialEventsEvent && !specialEventsRegistered) {
            g_specialEventsEvent = g_ctx.coreAPI->telemetry->Tel_RegisterForSpecialEvents(g_ctx.telemetryHandle, OnSpecialEventsUpdate, nullptr);
            specialEventsRegistered = true;
        }
    }

    void Unregister() {
        g_jobDataEvent = nullptr;
        g_specialEventsEvent = nullptr;
        jobDataRegistered = false;
        specialEventsRegistered = false;
    }
}
