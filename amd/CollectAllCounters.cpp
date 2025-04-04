//==================================================================================
// Copyright (c) 2016-2018 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CollectAllCounters.cpp
///
//==================================================================================

// - Start the profiling
// - Periodically read the counter values and report till the user has requested to stop
// - Uprof driver must be installed to use this example

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#ifdef __linux__
    #include <unistd.h>
#endif

// TODO: The header file needs to be removed
// Need modification in source program.
#include <AMDTPowerProfileApi.h>

#if defined(__linux__)
bool isDriverInstalled(const char* driverName)
{
    AMDTResult retVal = AMDT_STATUS_OK;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "lsmod | grep %s", driverName);

    //open PIPE to read the outputb of lsmod
    FILE* pipe = popen(cmd, "r");

    if(!pipe)
    {
        perror("popen");
        retVal = AMDT_ERROR_FAIL; //return false on error
    }

    //Read lsmod output if retVal is AMDT_STATUS_OK
    if (AMDT_STATUS_OK == retVal)
    {
        char buf[256];
        std::string result;

        while (fgets(buf, sizeof(buf), pipe) != nullptr)
        {
            result += buf;
        }

        pclose(pipe);

        //check if result contains the driverName or not
        retVal = result.find(driverName) != std::string::npos ? AMDT_STATUS_OK : AMDT_ERROR_DRIVER_UNAVAILABLE;
    }

    return (retVal == AMDT_STATUS_OK) ? true : false;
}
#endif

void CollectAllCounters()
{
    AMDTResult hResult = AMDT_STATUS_OK;

    // Initialize online mode
    hResult = AMDTPwrProfileInitialize(AMDT_PWR_MODE_TIMELINE_ONLINE);
    // --- Handle the error

    // Configure the profile run
    //   1. Get the supported counters
    //   2. Enable all the counter
    //   3. Set the timer configuration

    // 1. Get the supported counter details
    AMDTUInt32 nbrCounters = 0;
    AMDTPwrCounterDesc* pCounters = nullptr;

#if defined(__linux__)
    if(!isDriverInstalled("AMDPowerProfiler"))
    {
        fprintf(stderr, "ERROR: Driver not available. Please install the driver by running"
                            " 'AMDPowerProfilerDriver.sh install'\n");
        return;
    }
#endif

    hResult = AMDTPwrGetSupportedCounters(&nbrCounters, &pCounters);
    assert(AMDT_STATUS_OK == hResult);

    AMDTPwrCounterDesc* pCurrCounter = pCounters;

    for (AMDTUInt32 cnt = 0; cnt < nbrCounters; cnt++, pCurrCounter++)
    {
        if (nullptr != pCurrCounter)
        {
            // Enable all the counters
            hResult = AMDTPwrEnableCounter(pCurrCounter->m_counterID);
            assert(AMDT_STATUS_OK == hResult);
        }
    }

    // Set the timer configuration
    AMDTUInt32 samplingInterval = 1000;     // in milliseconds
    AMDTUInt32 profilingDuration = 10;      // in seconds

    hResult = AMDTPwrSetTimerSamplingPeriod(samplingInterval);
    assert(AMDT_STATUS_OK == hResult);

    // Start the Profile Run
    hResult = AMDTPwrStartProfiling();
    assert(AMDT_STATUS_OK == hResult);

    // Collect and report the counter values periodically
    //   1. Take the snapshot of the counter values
    //   2. Read the counter values
    //   3. Report the counter values

    volatile bool isProfiling = true;
    bool stopProfiling = false;
    AMDTUInt32 nbrSamples = 0;

    while (isProfiling)
    {
        // sleep for refresh duration - at least equivalent to the sampling interval specified
#if defined ( WIN32 )
        // Windows
        Sleep(samplingInterval);
#else
        // Linux
        usleep(samplingInterval * 1000);
#endif

        // read all the counter values
        AMDTPwrSample* pSampleData = nullptr;

        hResult = AMDTPwrReadAllEnabledCounters(&nbrSamples, &pSampleData);

        if (AMDT_STATUS_OK != hResult)
        {
            continue;
        }

        if (nullptr != pSampleData)
        {
            // iterate over all the samples and report the sampled counter values
            for (AMDTUInt32 idx = 0; idx < nbrSamples; idx++)
            {
                // Iterate over the sampled counter values and print
                for (unsigned int i = 0; i < pSampleData[idx].m_numOfCounter; i++)
                {
                    if (nullptr != pSampleData[idx].m_counterValues)
                    {
                        // Get the counter descriptor to print the counter name
                        AMDTPwrCounterDesc counterDesc;
                        AMDTPwrGetCounterDesc(pSampleData[idx].m_counterValues->m_counterID, &counterDesc);

                        fprintf(stdout, "%s : %f ", counterDesc.m_name, pSampleData[idx].m_counterValues->m_data);

                        pSampleData[idx].m_counterValues++;
                    }
                } // iterate over the sampled counters

                fprintf(stdout, "\n");
            } // iterate over all the samples collected

            // check if we exceeded the profile duration
            if ((profilingDuration > 0)
                && (pSampleData->m_elapsedTimeMs >= (profilingDuration * 1000)))
            {
                stopProfiling = true;
            }

            if (stopProfiling)
            {
                // stop the profiling
                hResult = AMDTPwrStopProfiling();
                assert(AMDT_STATUS_OK == hResult);
                isProfiling = false;
            }
        }
    }

    // Close the profiler
    hResult = AMDTPwrProfileClose();
    assert(AMDT_STATUS_OK == hResult);
}

int main()
{
    AMDTResult hResult = AMDT_STATUS_OK;
    CollectAllCounters();
    return hResult;
}
