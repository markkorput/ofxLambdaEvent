# ofxLambdaEvent
Bringing te power of lambda to ofEvent

## Credits
The core lambda logic is handled by the internal ```ofxLiquidEvent``` class, which is grabbed directly from [https://github.com/elliotwoods/ofxLiquidEvent](https://github.com/elliotwoods/ofxLiquidEvent) so all those credits go to elliotwoods (I just added the locking and modification queueing). The reason I copied his class instead of using his addon, is to keep the dependency list clean, and adding another addon dependency for a single file seems like an unnecessary pain in the ass.

## Explanation
This addon provides a ```LambdaEvent``` class which is basically nothing more than ofEvent and [ofxLiquidEvent](https://github.com/elliotwoods/ofxLiquidEvent) merged into one, with some logic to tie the two together and some extra convenience methods.

Note also that the ```LambdaEvent``` class has no smart logic to trigger the listeners in the proper order when both ofEvent-style listeners and ofxLiquidEvent-style lambdas are used. Mixing both types on the same event is possible but simply not recommended; use your preferred method consistently.

## Usage - lambda only
_You can use a LambdaEvent like an ofxLiquidEvent_

```c++
// create an event instance
LambdaEvent<SomeCustomObjectType> customEvent;
// register a listener;
// arg1 is the lambda that will be called when the event is triggered
// arg2 (this) is linked to the lambda so you can later unregister the lambda
customEvent.addListener([](SomeCustomObjectType obj){
    doSomethingWith(obj);
}, this);
// trigger the event
customEvent.notifyListeners(instanceOfSomeCustomObjectType);
// unregister lambda; this will remove ALL lambdas which were registered with 'this' as second argument
customEvent.removeListeners(this);
```

## Usage - like an ofEvent
_You can use LambdaEvent like an ofEvent_
```c++
// create an event instance
LambdaEvent<SomeCustomObjectType> customEvent;
// register listener, like you would do with ofEvent
ofAddListener(customEvent, this, &ThisClass::listenerMethod);
// trigger event
ofNotifyListeners(customEvent, instanceOfSomeCustomObjectType);
// unregister listener
ofRemoveListener(customEvent, this, &ThisClass::listenerMethod);
```

## Usage - using lambdas on an existing ofEvent
If you're using 3rd-party libraries (of core-openFrameworks events) you might have to deal with an existing ofEvent. The ```LambdaEvent``` class has a convenience method `forward` that provides an easy work-around to "add lambda support" to an existing ofEvent;

```c++
// create your own LambdaEvent with the same template-type as the existing ofEvent
LambdaEvent<ofKeyEventArgs> lambdafierEvent;
// let your lambda event "forward" all notifications from the original event
lambdafierEvent.forward(ofEvents().keyPressed);
// register your own lambda like on the LambdaEvent
lambdafierEvent.addListener([](ofKeyEventArgs args){
    ofLog() << "I'm never using ofAddListener again!";
}, this);

```

Note that, obviously, LambdaEvent's destructor cleans up after itself nicely, so it will stop forwarding when de-allocated. If for some reason you want to disconnect from the original ofEvent before that, this is also possible: ``` lambdafierEvent.stopForward(ofEvents().keyPressed); ```


## Extra - Middleware
The Middleware class has exactly the same API as ofxLiquidEvent (.addListener, .notifyListeners, .removeListeners), but takes lambdas that return a boolean value. If any of the listeners returns false, the notifications immediately end and remaining listeners are not invoked. The notifyListeners function also returns wether all lambdas returned true or not; this way the Middleware class can be used to have listeners inform the notifier about wether to perform a certain process or not.

```c++
Middleware<string> uploadMiddleware;

uploadMiddleware.addListener([](string& url) -> bool { return url.substring(0, 8) == "https://"; )}, this);

if(uploadMiddleware.notifyListeners(uploadUrlString)){
    // perform upload logic
}
```
