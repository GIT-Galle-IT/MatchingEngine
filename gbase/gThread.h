/*
    Mutithreading utility function
    Author - Chathura Weerasinghe
*/

#pragma once
#include <iostream>
#include <thread>
#include <pthread>
#include <unistd.h>
#include <atomic>
#include <chrono>

namespace gbase
{

    using namespace std::literals::chrono_literals;

    inline auto pinThreadToCore(int coreId) noexecpt
    {
        cpu_set_t cpuSet;
        CPU_ZERO(cpuSet); //make sure none of the cores are selected
        CPU_SET(coreId, &cpuSet); //set selected core to run on
        return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet) == 0); //set core affinity for the created thread
    }

    template<typename T, typename... A>
    iniline auto createStartThread(int coreId, T&& fn, A&& ...args)
    {
        std::atomic<bool> stateThreadRunning{false}, stateThreadFailed{false};

        auto threadBody = [&]()
        {
            if (!(coreId>=0 && pinThreadToCore(coreId)))
            {
                stateThreadFailed = true;
                return;
            }
            stateThreadRunning = true;
            std::forward<T>(fn)((std::forward<A>args)...);
        };

        auto thread = std::make_unique<std::thread>(threadBody);
        while (!(stateThreadRunning || stateThreadFailed))
        {
            std::this_thread::sleep_for(50ms);
        }

        if (stateThreadFailed)
        {
            thread.join();
            return nullptr;
        }
    }
}