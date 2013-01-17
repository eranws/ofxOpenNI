#pragma once
#include "ofEvents.h"
#include <deque>
#include "ofThread.h"
#include "NiTE.h"

namespace openni
{
	class Device;
}

class ofxHandTracker : public ofThread
{
public:
	void setup(ofPtr<openni::Device> device = ofPtr<openni::Device>(), bool isVerbose = false);
	void exit();

	ofPoint getHandPoint() const { return handPoint; }
	void readFrame();

protected:
	virtual void threadedFunction();
private:
	ofPtr<openni::Device> device;

	nite::HandTracker handTracker;		
	nite::HandTrackerFrameRef handTrackerFrame;	
	nite::Status niteRc;

	ofPoint handPoint;

	friend class GroundTruthReader;
};


