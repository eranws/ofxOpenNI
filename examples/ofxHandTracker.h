#pragma once
#include "ofEvents.h"
#include <deque>
#include "ofThread.h"

class Tracker {};
class TrackerEventArgs{};

// helper function
template<class ListenerClass>
void registerHandTrackerEvents(ListenerClass * listener){
	ofAddListener(getTrackerEvents().handUpdate, listener, &ListenerClass::handUpdate);
}

class TrackerEvents{
public:
	ofEvent<float> handVelocityUpdate;
	ofEvent<ofPoint> handUpdate;
	//TODO: more events here...
};

TrackerEvents& getTrackerEvents();

namespace openni
{
	class Device;
}

class ofxHandTracker : public ofThread
{

public:
	ofxHandTracker() : _historySize(8)
	{
	}

	void setup(ofPtr<openni::Device> device = ofPtr<openni::Device>(), bool isVerbose = false);
	void exit();

	ofPoint getHandPoint() const { return handPoint; }

	std::deque<ofPoint> positionHistory() const { return _positionHistory; }
	unsigned int historySize() const { return _historySize; }

protected:
	virtual void threadedFunction();

private:
	std::deque<ofPoint> _positionHistory;
	unsigned int _historySize;
	ofPtr<openni::Device> device;

	ofPoint handPoint;
};


