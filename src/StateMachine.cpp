//
//  StateMachine.cpp
//  SecondaryTaskPlugin
//
//  Created by Fernando Macedo on 20/10/2021.
//

#include "StateMachine.hpp"

#include <assert.h>
#include <cmath>
#include <functional>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <thread>

class Timer
{
    std::atomic<bool> running{false};

public:
    typedef std::chrono::milliseconds Interval;
    typedef std::function<void(void)> Timeout;

    void start(const Interval &interval,
               const Timeout &timeout)
    {
        running = true;

        std::thread t([=]() {
            if(!running.load()) { return; }
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            if(!running.load()) { return; }
            timeout();
        });
        t.detach();
    }

    void stop()
    {
        running = false;
    }
};

static const unsigned k_maxSignalSeconds = 25;
static const unsigned k_minSignalSeconds = 7;
static const unsigned k_responseTimeoutSeconds = 5;

static Timer s_signalTimeElapsedTimer;
static Timer s_responseTimeoutTimer;


#pragma mark - Auxiliary Functions

std::string stateToString(int state) {
    switch (state) {
        case State::WaitForStart:
            return "WaitForStart";
        case State::Idle:
            return "Idle";
        case State::SendSignal:
            return "SendSignal";
        case State::WaitResponse:
            return "WaitResponse";
        case State::ProcessResponse:
            return "ProcessResponse";
        default:
            return "Unrecognized State";
    }
}

std::string eventToString(int eventId) {
    switch (eventId) {
        case Event::StartMeasure:
            return "StartMeasure";
        case Event::SignalTimeElapsed:
            return "SignalTimeElapsed";
        case Event::SignalSent:
            return "SignalSent";
        case Event::ResponseReceived:
            return "ResponseReceived";
        case Event::ResponseTimeout:
            return "ResponseTimeout";
        case Event::ResponseProcessed:
            return "ResponseProcessed";
        default:
            return "Unrecognized Event";
    }
}

#pragma mark - State Machine

StateMachine& StateMachine::GetInstance() {
    static StateMachine instance;
    return instance;
}

StateMachine::StateMachine() {
    std::srand(static_cast<unsigned>(std::time(nullptr))); // Initialize random number generator.
    _shouldAddMilestone = true; // Start at true to create first milestone
    _shouldAddLogMilestone = true;
    _initialState = State::WaitForStart;
    _state = State::WaitForStart;
    setValidTransitions({
        // start
        StateMachine::Transition(Event::StartMeasure, State::WaitForStart, State::Idle),
        // default flow
        StateMachine::Transition(Event::SignalTimeElapsed, State::Idle, State::SendSignal),
        StateMachine::Transition(Event::SignalSent, State::SendSignal, State::WaitResponse),
        StateMachine::Transition(Event::ResponseReceived, State::WaitResponse, State::ProcessResponse),
        StateMachine::Transition(Event::ResponseProcessed, State::ProcessResponse, State::Idle),
        // response never came
        StateMachine::Transition(Event::ResponseTimeout, State::WaitResponse, State::ProcessResponse)
    });
}

void StateMachine::setValidTransitions(std::vector<Transition> validTransitions) {
    _validTransitions.clear();
    for (Transition& transition : validTransitions) {

        auto it = _validTransitions.find(transition.eventId);
        bool newEventType = it == _validTransitions.end();
            
        if(newEventType) {
            _validTransitions.emplace(transition.eventId, std::vector<Transition>{transition});
        } else {
            it->second.emplace_back(transition);
        }
    }
}

bool StateMachine::checkState(int validState) {
    return _state == validState;
}

void StateMachine::processEvent(int eventId) {
    auto it = _validTransitions.find(eventId);
    bool validEventFound = it != _validTransitions.end();
    bool validTransitionFound = false;
    
    if (validEventFound) {
        for (Transition& transition : it->second) {
            validTransitionFound = checkState(transition.validState);
            
            if (validTransitionFound) {
                debugLog("%s: %s -> %s", eventToString(eventId).c_str(), stateToString(_state).c_str(), stateToString(transition.nextState).c_str());
                _state = transition.nextState;
                processTransition(transition);
                break;
            }
        }
    }
   // assert(validTransitionFound);
    if (!validTransitionFound) {
        debugLog("Reached Assert State with event %s", eventToString(eventId).c_str());
    }
}

void StateMachine::processTransition(const Transition& transition) {
    switch (transition.nextState) {
        case State::WaitForStart: {
            debugLog("Reached WaitForStart State");
            break;
        }
        case State::Idle: {
            debugLog("Reached Idle State");
            if (transition.validState == State::WaitForStart) {
                _startMeasuringTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            }
            int randsecs = std::rand()%(k_maxSignalSeconds-k_minSignalSeconds + 1) + k_minSignalSeconds;
            s_signalTimeElapsedTimer.start(std::chrono::milliseconds(randsecs * 1000), []{
                    StateMachine::GetInstance().processEvent(Event::SignalTimeElapsed);
                });
            break;
        }
        case State::SendSignal: {
            debugLog("Reached SendSignal State");
            s_signalTimeElapsedTimer.stop();
            (*_signalSendingCallback)();
            _sentSignalTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            processEvent(Event::SignalSent);
            break;
        }
        case State::WaitResponse: {
            debugLog("Reached WaitResponse State");
            s_responseTimeoutTimer.start(std::chrono::milliseconds(k_responseTimeoutSeconds * 1000), []{
                    StateMachine::GetInstance().processEvent(Event::ResponseTimeout);
                });
            break;
        }
        case State::ProcessResponse: {
            debugLog("Reached Process Response State");
            s_responseTimeoutTimer.stop();
            long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            long msSinceStart = now - _startMeasuringTimestamp.count();
            long msReactionTime = now - _sentSignalTimestamp.count();
            if (msReactionTime < 100) { // if reaction time is lower then the limit of human reaction time then we record it the same as having missed the stimulus
                msReactionTime = k_responseTimeoutSeconds * 1000;
            }
            (*_signalStopCallback)();
            if (_shouldAddMilestone) {
                debugLog("MileStone Added");
                _shouldAddMilestone = false;
                std::map<long,long> m;
                m[msSinceStart] = msReactionTime;
                _collectedData.first.emplace_back(m);
            } else {
                _collectedData.first.back().emplace(msSinceStart, msReactionTime);
            }
            debugLog("ms from start: %d, ms reaction:%d", msSinceStart, msReactionTime);
            processEvent(Event::ResponseProcessed);
            break;
        }
    }
}

// StopMeasure
void StateMachine::resetState() {
    s_signalTimeElapsedTimer.stop();
    s_responseTimeoutTimer.stop();
    debugLog("RESET: %s -> %s", stateToString(_state).c_str(), stateToString(_initialState).c_str());
    _state = _initialState;
    _shouldAddMilestone = true; // Start at true to create first milestone
    _shouldAddLogMilestone = true;
    _collectedData.first.clear();
    _collectedData.second.clear();
}

// MileStone Reached
void StateMachine::addMilestone() {
    _shouldAddMilestone = true;
    _shouldAddLogMilestone = true;
}

void StateMachine::setSignalSendingCallback(void (*callback)()) {
    _signalSendingCallback = callback;
}

void StateMachine::setSignalStopCallback(void (*callback)()) {
    _signalStopCallback = callback;
}

void StateMachine::addLogEvent(std::string eventName) {
    if (_state == State::WaitForStart) {
        return;
    }
    
    long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    long msSinceStart = now - _startMeasuringTimestamp.count();
    if (_shouldAddLogMilestone) {
        debugLog("MileStone Added");
        _shouldAddLogMilestone = false;
        std::map<long,std::string> m;
        m[msSinceStart] = eventName;
        _collectedData.second.emplace_back(m);
    } else {
        _collectedData.second.back().emplace(msSinceStart, eventName);
    }
}

void StateMachine::setDebugLogCallback(void (*callback)(const char *)) {
    _debugLogCallback = callback;
}

void StateMachine::debugLog(const char* str, ...) {
    if (_debugLogCallback != nullptr && !(str == nullptr || std::strlen(str) == 0)) {
        va_list args;
        va_start(args, str);
        
        va_list argsCopy;
        va_copy(argsCopy, args);

        auto bufferSize = vsnprintf(nullptr, 0, str, args);

        if (bufferSize <= 0) {
            va_end(argsCopy);
            va_end(args);
            return;
        }
        
        char* buffer = (char*)malloc((size_t)bufferSize + 1); // + 1 for string delimiter
#ifndef MAC_BUILD
        auto writtenBytes = vsprintf_s(buffer, (size_t)bufferSize + 1, str, argsCopy);
#else
        auto writtenBytes = vsprintf(buffer, str, argsCopy);
#endif
        
        if (writtenBytes != bufferSize) {
            free(buffer);
            buffer = nullptr;
            
            va_end(argsCopy);
            va_end(args);
            return;
        }
        
        std::string result(buffer);
        
        free(buffer);
        buffer = nullptr;
        
        (*_debugLogCallback)(result.c_str());
        va_end(argsCopy);
        va_end(args);
    }
}
