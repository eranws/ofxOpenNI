#pragma once
#include "ofThread.h"

namespace openni
{
	class Device;
}

class ofxDepthStream : public ofThread
{

public:

	void setup();
	void exit();

	openni::Device* getDevice() const { return device; }

protected:
	virtual void threadedFunction();

	openni::Device* device;
	
};
