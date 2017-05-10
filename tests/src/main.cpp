#include "ofxUnitTests.h"
#include "ofxLambdaEvent.h"

#define TEST_START(x) {ofLog()<<"CASE: "<<#x;
#define TEST_END }

class ofApp: public ofxUnitTestsApp{
    string attribute1;

    void _onEvent1(string& param){
        attribute1 += param;
    }

    string attribute2;

    void _onEvent2(string& param){
        attribute2 += param;
    }

    void run(){
        TEST_START(lambda listener)
            LambdaEvent<string> event;

            string result;
            string arg = "ciao mondo";

            // add listener
            event.addListener([&result](string& param){ result += param; }, this);
            // trigger event
            event.notifyListeners(arg);
            // verify listener was called
            test_eq(result, "ciao mondo", "");
            // trigger event again
            event.notifyListeners(arg);
            // verify listener was called again
            test_eq(result, "ciao mondociao mondo", "");
            // remove listener
            event.removeListeners(this);
            // trigger event for third time
            event.notifyListeners(arg);
            // verify listener was NOT called for the third time...
            test_eq(result, "ciao mondociao mondo", "");
        TEST_END

        TEST_START(traditional listener)
            LambdaEvent<string> event;

            attribute1="";
            string arg = "traditional listener works!";

            // add listener
            ofAddListener(event, this, &ofApp::_onEvent1);
            // trigger event
            ofNotifyEvent(event, arg);
            // verify listener was called
            test_eq(attribute1, "traditional listener works!", "");
            // trigger event again
            ofNotifyEvent(event, arg);
            // verify listener was called again
            test_eq(attribute1, "traditional listener works!traditional listener works!", "");
            // remove listener
            ofRemoveListener(event, this, &ofApp::_onEvent1);
            // trigger event for third time
            ofNotifyEvent(event, arg);
            // verify listener was NOT called for the third time...
            test_eq(attribute1, "traditional listener works!traditional listener works!", "");
        TEST_END

        TEST_START(forward LambdaEvent)
            LambdaEvent<string> eventA, eventB;
            eventB.forward(eventA);

            string result;
            string param = "forward!";

            eventB.addListener([&result](string& arg){ result += arg; }, this);
            eventA.notifyListeners(param);
            test_eq(result, param, "");
            ofNotifyEvent(eventA, param);
            test_eq(result, param+param, "");
        TEST_END

        TEST_START(forward ofEvent)
            ofEvent<string> eventA;
            string result;
            string param = "forward!";

            {
                LambdaEvent<string> eventB;
                eventB.forward(eventA);
                test_eq(eventA.size(), 1, "");
                eventB.addListener([&result](string& arg){ result += arg; }, this);
                ofNotifyEvent(eventA, param);
                test_eq(result, param, "");
            }

            // test if the de-allocatd eventB cleaned up nicely
            test_eq(eventA.size(), 0, "");
        TEST_END

        TEST_START(forward to ofEvent)
            LambdaEvent<string> eventA;
            ofEvent<string> eventB;
            string param = "take this!";

            eventA.forwardTo(eventB);
            ofAddListener(eventB, this, &ofApp::_onEvent2);

            eventA.notifyListeners(param);
            test_eq(attribute2, param, "");
            ofNotifyEvent(eventA, param);
            test_eq(attribute2, param+param, "");
            test_eq(eventA.size(), 1, "");
            eventA.stopForwardTo(eventB);
            test_eq(eventA.size(), 0, "");
        TEST_END
    }
};


#include "ofAppNoWindow.h"
#include "ofAppRunner.h"

int main( ){
  ofInit();
  auto window = std::make_shared<ofAppNoWindow>();
  auto app = std::make_shared<ofApp>();
  ofRunApp(window, app);
  return ofRunMainLoop();
}
