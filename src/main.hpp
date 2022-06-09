//
//  main.hpp
//  SecondaryTaskPlugin
//
//  Created by Fernando Macedo on 18/10/2021.
//

#ifndef main_hpp
#define main_hpp

extern "C"
{
#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    void initializeSecondaryTaskWithStimulusHandler(void (*signalHandler)(), void (*signalStopHandler)(), void (*debugLogHandler)(const char*));
#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    void startMeasurement();
#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    void respondToStimulus();
#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    void stopMeasurement();
#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    void addMilestone();
#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    void addEventLog(const char* eventName);

#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    char* exportReactionData();

#ifndef MAC_BUILD
    __declspec(dllexport)
#endif
    char* exportEventsData();
}
#endif /* main_hpp */
