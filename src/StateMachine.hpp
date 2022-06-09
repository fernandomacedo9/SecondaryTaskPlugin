//
//  StateMachine.hpp
//  SecondaryTaskPlugin
//
//  Created by Fernando Macedo on 20/10/2021.
//

#ifndef StateMachine_hpp
#define StateMachine_hpp

#include <chrono>
#include <iostream>
#include <map>
#include <vector>

struct State {
    enum {
        WaitForStart,
        Idle,
        SendSignal,
        WaitResponse,
        ProcessResponse
    };
};

struct Event {
    enum {
        StartMeasure,
        SignalTimeElapsed,
        SignalSent,
        ResponseReceived,
        ResponseTimeout,
        ResponseProcessed
    };
};

class StateMachine {
public:
    struct Transition {
        const int eventId;
        const int validState;
        const int nextState;
        
        Transition(int _eventId, int _validState, int _nextState) :
            eventId(_eventId), validState(_validState), nextState(_nextState) {}
    };
    
    static StateMachine& GetInstance();
    
    // delete copy and move constructors and assign operators
    StateMachine(StateMachine const&) = delete;             // Copy construct
    StateMachine(StateMachine&&) = delete;                  // Move construct
    StateMachine& operator=(StateMachine const&) = delete;  // Copy assign
    StateMachine& operator=(StateMachine &&) = delete;      // Move assign
    
    
    bool checkState(int validState);
    void processEvent(int eventId);
    
    void resetState();
    void addMilestone();
    
    void setSignalSendingCallback(void (*callback)());
    void setSignalStopCallback(void (*callback)());
    void setDebugLogCallback(void (*callback)(const char *));
    
    void addLogEvent(std::string eventName);
    
    std::pair<std::vector<std::map<long,long>>, std::vector<std::map<long,std::string>>> getCollectedData() {return _collectedData;};
    
protected:
    StateMachine();
    ~StateMachine(){}
    
private:
    void setValidTransitions(std::vector<Transition> validTransitions);
    void processTransition(const Transition& transition);
    void debugLog(const char *, ...);
    
private:
    std::map<int, std::vector<Transition>> _validTransitions;
    
    int _initialState;
    int _state;
    bool _shouldAddMilestone;
    bool _shouldAddLogMilestone;
    void (*_signalSendingCallback)();
    void (*_signalStopCallback)();
    void (*_debugLogCallback)(const char *);
    
    std::chrono::milliseconds _startMeasuringTimestamp;
    std::chrono::milliseconds _sentSignalTimestamp;
    std::pair<std::vector<std::map<long,long>>, std::vector<std::map<long,std::string>>> _collectedData;
};

#endif /* StateMachine_hpp */
