// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C"
{

    void initializeSecondaryTaskWithStimulusHandler(void (*signalHandler)(), void (*signalStopHandler)(), void (*debugLogHandler)(const char*)) {
        StateMachine::GetInstance().setDebugLogCallback(debugLogHandler);
        StateMachine::GetInstance().setSignalSendingCallback(signalHandler);
        StateMachine::GetInstance().setSignalStopCallback(signalStopHandler);
    }

    void startMeasurement() {
        StateMachine::GetInstance().resetState();
        StateMachine::GetInstance().processEvent(Event::StartMeasure);
    }

    void respondToStimulus() {
        StateMachine::GetInstance().processEvent(Event::ResponseReceived);
    }

    void stopMeasurement() {
        StateMachine::GetInstance().resetState();
    }

    void addMilestone() {
        StateMachine::GetInstance().addMilestone();
    }

    char* exportData() {
        std::vector<std::map<long, long>> reactionTimes = StateMachine::GetInstance().getReactionTimes();
        std::string result = "[";

        for (int i = 0; i < reactionTimes.size(); i++) {
            result += "[" + std::to_string(i) + ",";
            for (auto iter = reactionTimes[i].begin(); iter != reactionTimes[i].end(); ) {
                result += "[" + std::to_string(iter->first) + "," + std::to_string(iter->second) + "]";
                if (++iter != reactionTimes[i].end()) {
                    result += ",";
                }
            }
            if (i == reactionTimes.size() - 1) {
                result += "]";
            }
            else {
                result += "],";
            }
        }

        result += "]";

        //create a null terminated C string on the heap so that our string's memory isn't wiped out right after method's return
        char* cString = (char*)malloc(strlen(result.c_str()) + 1);
        strcpy_s(cString, sizeof cString, result.c_str());

        return cString;
    }

}

