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
    inline bool notify(Type & param);
    inline bool notify(const void* sender, Type & param);

public: // ofxLiquidEvent methods
    bool notifyListeners();
    bool notifyListeners(Type& arguments);
    bool notifyListenersInReverse(Type& arguments);

public: // custom methods
    void forward(ofEvent<Type> &event);
    void stopForward(ofEvent<Type> &event);

    void forward(LambdaEvent<Type> &event);
    void stopForward(LambdaEvent<Type> &event);

    //! stops forwarding all events
    void stopForward();

    void forwardTo(ofEvent<Type> &event);
    void stopForwardTo(ofEvent<Type> &event);

    size_t size() const {
        return ofEvent<Type>::size() + ofxLiquidEvent<Type>::size();
    }

private: // callbacks
    void onForwardEvent(Type& arg);

private: // attributes
    std::vector<LambdaEvent<Type>*> forwardFromLambdaEvents;
    std::vector<ofEvent<Type>*> forwardFromOfEvents;
};



template<typename Type>
inline void LambdaEvent<Type>::destroy(){
    stopForward();
}

// overwrite ofEvent method; call original implementation AND call lambda notifications
template<typename Type>
inline bool LambdaEvent<Type>::notify(Type & param){
    ofxLiquidEvent<Type>::notifyListeners(param);
    return ofEvent<Type>::notify(param);
}

// overwrite ofEvent method; call original implementation AND call lambda notifications
template<typename Type>
inline bool LambdaEvent<Type>::notify(const void* sender, Type & param){
    ofxLiquidEvent<Type>::notifyListeners(param);
    return ofEvent<Type>::notify(sender, param);
}

// overwrite ofxLiquidEvent method; call original implementation AND call ofEvent notifications
template<typename Type>
bool LambdaEvent<Type>::notifyListeners(Type& arguments){
    ofxLiquidEvent<Type>::notifyListeners(arguments);
    return ofEvent<Type>::notify(this, arguments);
}

// overwrite ofxLiquidEvent method; call original implementation AND call ofEvent notifications
template<typename Type>
bool LambdaEvent<Type>::notifyListeners(){
    ofxLiquidEvent<Type>::notifyListeners();
    return ofEvent<Type>::notify();
}

// overwrite ofxLiquidEvent method; call original implementation AND call ofEvent notifications
template<typename Type>
bool LambdaEvent<Type>::notifyListenersInReverse(Type& arguments){
    ofxLiquidEvent<Type>::notifyListenersInReverse(arguments);
    return ofEvent<Type>::notify(arguments);
}

template<typename Type>
void LambdaEvent<Type>::forward(ofEvent<Type> &event){
    ofAddListener(event, this, &LambdaEvent<Type>::onForwardEvent);
    // keep a record of events we're following
    forwardFromOfEvents.push_back(&event);
}


template<typename Type>
void LambdaEvent<Type>::stopForward(ofEvent<Type> &event){
    ofRemoveListener(event, this, &LambdaEvent<Type>::onForwardEvent);

    for(auto it = forwardFromOfEvents.begin(); it != forwardFromOfEvents.end(); it++){
        if((*it) == &event){
            // ofLog() << "stopForward: found";
            forwardFromOfEvents.erase(it);
            return;
        }
    }

    ofLogWarning() << "couldn't find ofEvent instance to stop forwarding from";
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
void LambdaEvent<Type>::stopForward(){
    for(auto event : forwardFromLambdaEvents)
        stopForward(*event);
    forwardFromLambdaEvents.clear();

    for(auto ofevent : forwardFromOfEvents)
        ofRemoveListener(*ofevent, this, &LambdaEvent<Type>::onForwardEvent);
    forwardFromOfEvents.clear();
}

template<typename Type>
void LambdaEvent<Type>::forwardTo(ofEvent<Type> &event){
    this->addListener([&event](Type& value){
        ofNotifyEvent(event, value);
    }, &event);
}

template<typename Type>
void LambdaEvent<Type>::stopForwardTo(ofEvent<Type> &event){
    this->removeListeners(&event);
}

template<typename Type>
void LambdaEvent<Type>::onForwardEvent(Type& arg){
    notifyListeners(arg);
}
