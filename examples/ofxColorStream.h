#pragma once
#include "ofThread.h"

namespace openni
{
	class Device;
	class VideoStream;
}

class ofxColorStream : public ofThread
{

public:

	void setup(ofPtr<openni::Device> device = ofPtr<openni::Device>());
	void exit();

	ofPtr<openni::Device> getDevice() const { return device; }

protected:
	virtual void threadedFunction();

	ofPtr<openni::Device> device;
	ofPtr<openni::VideoStream> stream;

};
