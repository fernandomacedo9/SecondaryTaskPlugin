//
//  main.cpp
//  SecondaryTaskPlugin
//
//  Created by Fernando Macedo on 18/10/2021.
//

#include "main.hpp"

#include "StateMachine.hpp"

extern "C"
{

void initializeSecondaryTaskWithStimulusHandler(void (*handler)(), void (*debugLogHandler)(const char *)) {
    StateMachine::GetInstance().setDebugLogCallback(debugLogHandler);
    StateMachine::GetInstance().setSignalSendingCallback(handler);
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

}
