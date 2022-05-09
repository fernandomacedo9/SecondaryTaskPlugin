// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include "StateMachine.hpp"

#include <string>

extern "C"
{
    __declspec(dllexport) void initializeSecondaryTaskWithStimulusHandler(void (*signalHandler)(), void (*signalStopHandler)(), void (*debugLogHandler)(const char*));
    __declspec(dllexport) void startMeasurement();
    __declspec(dllexport) void respondToStimulus();
    __declspec(dllexport) void stopMeasurement();
    __declspec(dllexport) void addMilestone();

    __declspec(dllexport) char* exportData();
}

#endif //PCH_H
