#include "ofxUnitTests.h"
#include "ofxLambdaEvent.h"

#define TEST_START(x) {ofLog()<<"CASE: "<<#x;
#define TEST_END }

class ofApp: public ofxUnitTestsApp{

    void run(){
        TEST_START(lambda listener)
            LambdaEvent<string> event;

            string result;
            event.addListener([&result](string& param){
                result += param;
            }, this);

            string arg = "ciao mondo";
            event.notifyListeners(arg);
            test_eq(result, "ciao mondo", "");
            event.notifyListeners(arg);
            test_eq(result, "ciao mondociao mondo", "");
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
