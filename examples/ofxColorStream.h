#pragma once
#include "ofThread.h"
#include "ofTexture.h"

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
	ofPtr<ofPixels> getPixels() const { return pixels[0]; }
	

protected:

	ofTexture texture;
	ofPtr<ofPixels> pixels[2];
	
	virtual void threadedFunction();
	void allocateBuffers();
	ofPtr<openni::Device> device;
	ofPtr<openni::VideoStream> stream;

};
