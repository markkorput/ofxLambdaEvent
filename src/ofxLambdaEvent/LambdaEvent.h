#pragma once

#include "ofEvent.h"
#include "ofEventUtils.h"
#include "ofxLiquidEvent.h"

// LambdaEvent basically just merges ofEvent and ofxLiquidEvent into one, so people
// can use their preferred method (lambda or instance/method listener) to hook into the event
template<typename Type>
class LambdaEvent : public ofEvent<Type>, public ofxLiquidEvent<Type>  {

public: // constructor etc.
    ~LambdaEvent(){ destroy(); }
    void destroy();

public: // ofEvent method
    inline void notify(const void* sender, Type & param);

public: // ofxLiquidEvent methods
    void notifyListeners();
    void notifyListeners(Type& arguments);
    void notifyListenersInReverse(Type& arguments);

public: // custom methods
    void forward(LambdaEvent<Type> &event);
    void stopForward(LambdaEvent<Type> &event);

    void forwardTo(ofEvent<Type> &event);
    void stopForwardTo(ofEvent<Type> &event);

private: // attributes
    std::vector<ofEvent<Type>*> forwardToEvents;
    std::vector<LambdaEvent<Type>*> forwardFromLambdaEvents;
};



template<typename Type>
inline void LambdaEvent<Type>::destroy(){
    for(auto event : forwardFromLambdaEvents)
        stopForward(*event);

    forwardFromLambdaEvents.clear();
    forwardToEvents.clear();
}

// overwrite ofEvent method; call original implementation AND call lambda notifications
template<typename Type>
inline void LambdaEvent<Type>::notify(const void* sender, Type & param){
    ofEvent<Type>::notify(sender, param);
    ofxLiquidEvent<Type>::notifyListeners(param);
}

// overwrite ofxLiquidEvent method; call original implementation AND call ofEvent notifications
template<typename Type>
void LambdaEvent<Type>::notifyListeners(Type& arguments){
    ofxLiquidEvent<Type>::notifyListeners(arguments);
    ofEvent<Type>::notify(this, arguments);
}

// overwrite ofxLiquidEvent method; call original implementation AND call ofEvent notifications
template<typename Type>
void LambdaEvent<Type>::notifyListeners(){
    ofxLiquidEvent<Type>::notifyListeners();
    ofEvent<Type>::notify();
}

// overwrite ofxLiquidEvent method; call original implementation AND call ofEvent notifications
template<typename Type>
void LambdaEvent<Type>::notifyListenersInReverse(Type& arguments){
    ofxLiquidEvent<Type>::notifyListenersInReverse(arguments);
    ofEvent<Type>::notify(arguments);
}

template<typename Type>
void LambdaEvent<Type>::forward(LambdaEvent<Type> &event){
    event.addListener([this](Type& instance){ notifyListeners(instance); }, this);
    // keep a record of events we're following
    forwardFromLambdaEvents.push_back(&event);
}

template<typename Type>
void LambdaEvent<Type>::stopForward(LambdaEvent<Type> &event){
    event.removeListeners(this);
    for(auto it = forwardFromLambdaEvents.begin(); it != forwardFromLambdaEvents.end(); it++){
        if((*it) == &event){
            // ofLog() << "stopForward: found";
            forwardFromLambdaEvents.erase(it);
            return;
        }
    }

    ofLogWarning() << "couldn't find LambdaEvent instance to stop forwarding from";
}

template<typename Type>
void LambdaEvent<Type>::forwardTo(ofEvent<Type> &event){
    forwardToEvents.push_back(&event);
}

template<typename Type>
void LambdaEvent<Type>::stopForwardTo(ofEvent<Type> &event){
    for(auto it = forwardToEvents.begin(); it != forwardToEvents.end(); it++)
        if((*it) == &event)
            forwardToEvents.erase(it);
}
