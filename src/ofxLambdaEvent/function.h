#pragma once

// THIS IS BORROWED FROM https://github.com/elliotwoods/ofxLiquidEvent

//#if __cplusplus<201103L
//	#define OFXLAMBDAEVENT_USE_TR1
//#endif

#ifdef OFXLAMBDAEVENT_USE_TR1
	#include <tr1/functional>
	#define FUNCTION tr1::function
#else
	#include <functional>
	#define FUNCTION std::function
#endif
