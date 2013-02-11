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

	bool hasHand() {return !handTrackerFrame->getHands().isEmpty();} //HACKHACK awful Hack!
	int getFrameIndex() const { return frameIndex; }
	uint64_t getTimestamp() const { return timestamp; }


	void readFrame();

protected:
	virtual void threadedFunction();

private:
	ofPtr<openni::Device> device;

	nite::HandTracker handTracker;		
	ofPtr<nite::HandTrackerFrameRef> handTrackerFrame;	
	nite::Status niteRc;

	ofPoint handPoint;

	int frameIndex;
	uint64_t timestamp;
	friend class GroundTruthReader;
};


