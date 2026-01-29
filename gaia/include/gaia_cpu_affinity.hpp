#pragma once

#include "gaia_json_util.hpp"
#include "gaia_log.hpp"
#include <cstdint>
#include <thread>

namespace vp
{

struct Affinity
{
    bool enable = false;
    uint32_t cpu_index = 0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Affinity, enable, cpu_index)

inline bool setCpuAffinity(std::thread &thread, Affinity affinity)
{
#ifdef ANDROID
    // Do nothing. pthread_setaffinity_np does not work on Android.
    // If needed, use sched_setaffinity API instead
#else
    if (affinity.enable)
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(affinity.cpu_index, &cpuset); // NOLINT: C-API

        int rc = pthread_setaffinity_np(thread.native_handle(),
                                        sizeof(cpu_set_t), &cpuset);
        if (rc != 0)
        {
            LOG_WRN("Error calling pthread_setaffinity_np: {}", rc);
            return false;
        }
    }

    return true;
#endif
}

} // namespace vp