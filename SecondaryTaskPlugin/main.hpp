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
    void initializeSecondaryTaskWithStimulusHandler(void (*signalHandler)(), void (*signalStopHandler)(), void (*debugLogHandler)(const char *));
    void startMeasurement();
    void respondToStimulus();
    void stopMeasurement();
    void addMilestone();
}

#endif /* main_hpp */
