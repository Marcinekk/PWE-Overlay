#pragma once

namespace PWE::Events::Job {
    void Register();
    void Unregister();

    extern bool jobDataRegistered;
    extern bool specialEventsRegistered;
}