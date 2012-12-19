#pragma once
#include "ofEvents.h"
#include <deque>

class Tracker {};

class TrackerEventArgs{};

// helper function
template<class ListenerClass>
void registerHandTrackerEvents(ListenerClass * listener){
	ofAddListener(getTrackerEvents().handUpdate, listener, &ListenerClass::velocityUpdate);
}

class TrackerEvents{
public:
	//	ofEvent<float> velocityUpdate;
	ofEvent<TrackerEventArgs> handUpdate;
	//TODO: more events here...
};

TrackerEvents& getTrackerEvents();

namespace nite
{
	class HandTracker;
}

class ofxHandTracker 
{

public:
	ofxHandTracker() : _historySize(8) {
	}

	void setup();
	nite::HandTracker* handTracker;



	std::deque<ofPoint> positionHistory() const { return _positionHistory; }
	unsigned int historySize() const { return _historySize; }

private:
	std::deque<ofPoint> _positionHistory;
	unsigned int _historySize;
};


