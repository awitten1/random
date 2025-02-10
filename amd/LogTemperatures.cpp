
#include <AMDTPowerProfileApi.h>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <csignal>
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <thread>
#include "AMDTDefinitions.h"
#include "AMDTPowerProfileDataTypes.h"
#include "enum_utils.hpp"



AMDTUInt32 GetTemperatureCounterId() {

    AMDTUInt32 nbrCounters = 0;
    AMDTPwrCounterDesc* pCounters = nullptr;

    AMDTResult hResult = AMDTPwrGetSupportedCounters(&nbrCounters, &pCounters);
    for (int i = 0; i < nbrCounters; ++i) {
        std::string counter_name = pCounters[i].m_name;
        std::transform(counter_name.begin(), counter_name.end(), counter_name.begin(), 
            [](char c) {
                return std::tolower(c);
            });
        if (counter_name.find("temperature") != std::string::npos) {
            std::cout << counter_name << " " << pCounters[i].m_counterID << std::endl;
            return pCounters[i].m_counterID;
        }
    }
    throw std::runtime_error{"could not find temperature counter"};
}

std::atomic<bool> isProfiling = true;

void stopProfiling(int signum) {
    std::cout << "got signal " << signum << std::endl;
    isProfiling = false;
}

void Cleanup() {
    std::cout << "stopping profiler" << std::endl;
    AMDTResult hResult = AMDTPwrStopProfiling();
    assert(AMDT_STATUS_OK == hResult);

    // Close the profiler
    hResult = AMDTPwrProfileClose();
    assert(AMDT_STATUS_OK == hResult);

}


int main() {

    AMDTResult hResult = AMDTPwrProfileInitialize(AMDT_PWR_MODE_TIMELINE_ONLINE);
    if (hResult != AMDT_STATUS_OK) {
        AMDTPwrProfileClose();
        return hResult;
    }

    auto temperature_counter_id = GetTemperatureCounterId();

    signal( SIGINT, stopProfiling);

    hResult = AMDTPwrEnableCounter(temperature_counter_id);
    if (hResult != AMDT_STATUS_OK) {
        Cleanup();
        return hResult;
    }

    auto samplingInterval = std::chrono::milliseconds(1000);

    hResult = AMDTPwrSetTimerSamplingPeriod(samplingInterval.count());
    if (hResult != AMDT_STATUS_OK) {
        Cleanup();
        return hResult;
    }

    hResult = AMDTPwrStartProfiling();
    if (hResult != AMDT_STATUS_OK) {
        Cleanup();
        return hResult;
    }

    bool stopProfiling = false;
    AMDTUInt32 nbrSamples = 0;

    while(isProfiling) {
        std::this_thread::sleep_for(samplingInterval);
        AMDTUInt32 num_samples;
        AMDTPwrSample* samples;
        hResult = AMDTPwrReadAllEnabledCounters(&num_samples, &samples);
        if (hResult != AMDT_STATUS_OK) {
            Cleanup();
            return hResult;
        }

        auto& sample = samples[0];

        float temp = sample.m_counterValues->m_data;
        std::cout << temp << std::endl;
    }

    Cleanup();
    
    return 0;
}