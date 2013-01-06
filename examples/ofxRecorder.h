#pragma once
#include "ofThread.h"
#include "ofPixels.h"

namespace openni
{
	class Device;
	class VideoStream;
	class Recorder;
}

class ofxRecorder : public ofThread
{

public:

	void setup(ofPtr<openni::VideoStream> stream = ofPtr<openni::VideoStream>());

	void attach(ofPtr<openni::VideoStream> stream);
	void start();
	void stop();

	void exit();

	ofPtr<openni::Recorder> getRecorder() const { return recorder; }
	
protected:
	
	ofPtr<openni::VideoStream> _stream; //make a list?
	ofPtr<openni::Recorder> recorder;


};
