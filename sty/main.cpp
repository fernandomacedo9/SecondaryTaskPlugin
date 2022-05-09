//
//  main.cpp
//  sty
//
//  Created by Fernando Macedo on 04/11/2021.
//

#include <iostream>
#include <chrono>
#include <thread>

#include "StateMachine.hpp"

void logFunc(const char* text) {
    printf(text);
    printf("\n");
}

void handlerFunc() {
    printf("Signal Received\n");
}

void stopHandlerFunc() {
    printf("Signal Stop\n");
}

char* exportData() {
    std::vector<std::map<long,long>> reactionTimes = StateMachine::GetInstance().getReactionTimes();
    std::string result = "[";
    
    for (int i = 0; i < reactionTimes.size(); i++) {
        result += "[" + std::to_string(i) + ",";
        for (auto iter = reactionTimes[i].begin(); iter != reactionTimes[i].end(); ) {
            result += "[" + std::to_string(iter->first) + "," + std::to_string(iter->second) + "]";
            if (++iter != reactionTimes[i].end()) {
                result += ",";
            }
        }
        if (i == reactionTimes.size()-1) {
            result += "]";
        } else {
            result += "],";
        }
    }
    
    result += "]";
    
    //create a null terminated C string on the heap so that our string's memory isn't wiped out right after method's return
    char* cString = (char*)malloc(strlen(result.c_str()) + 1);
    strcpy(cString, result.c_str());

    return cString;
}

int main(int argc, const char * argv[]) {
    StateMachine::GetInstance().setDebugLogCallback(logFunc);
    StateMachine::GetInstance().setSignalSendingCallback(handlerFunc);
    StateMachine::GetInstance().setSignalStopCallback(stopHandlerFunc);
    StateMachine::GetInstance().resetState();
    StateMachine::GetInstance().processEvent(Event::StartMeasure);
    std::this_thread::sleep_for(std::chrono::seconds(30));
    auto data = exportData();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}
