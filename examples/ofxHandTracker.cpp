#include "ofxHandTracker.h"

#include "Nite.h"

#pragma once
#include "ofEvents.h"
#include <deque>

nite::HandTracker::Listener* handTrackerListener;
class ofxHandTrackerListener : nite::HandTracker::Listener
{
	virtual void onNewFrame( nite::HandTracker& niteHandTracker ) 
	{
		using namespace nite;

		HandTrackerFrameRef handFrame;
		niteHandTracker.readFrame(&handFrame);

		printf("%d", handFrame.getFrameIndex());

		handFrame.getHands();
		/*
		//raw event input
		_positionHistory.push_front(ofPoint(float(e.x),float(e.y)));
		if (_positionHistory.size() <= _historySize)
		{
		return;
		}
		else
		{
		_positionHistory.pop_back();
		}

		//process

		// example: calculate speed
		ofPoint speed = _positionHistory.back() - _positionHistory.front();
		float velocity = speed.length();
		ofNotifyEvent(getTrackerEvents().velocityUpdate, velocity); //TODO send id
		*/

		// example of processing: pointInverter


		//send Event

	}


};

TrackerEvents& getTrackerEvents()
{
	static TrackerEvents* events = new TrackerEvents;
	return *events;
}

TrackerEventArgs bangEventArgs;





void ofxHandTracker::setup()
{

	nite::Status niteRc;
	niteRc = nite::NiTE::initialize();
	if (niteRc != nite::STATUS_OK)
	{
		throw ("NiTE initialization failed\n");
	}

	handTracker = new nite::HandTracker;
	niteRc = handTracker->create();
	if (niteRc != nite::STATUS_OK)
	{
		throw ("Couldn't create user tracker\n");
	}

	handTracker->startGestureDetection(nite::GESTURE_WAVE);
	handTracker->startGestureDetection(nite::GESTURE_CLICK);

	handTracker->addListener(handTrackerListener);
}


//////////////////////////////////////////////////////////////////////////
// nite handTracker Sample:

/*
nite::HandTrackerFrameRef handTrackerFrame;
while (!wasKeyboardHit())
{
	niteRc = handTracker.readFrame(&handTrackerFrame);
	if (niteRc != nite::STATUS_OK)
	{
		printf("Get next frame failed\n");
		continue;
	}

	const nite::Array<nite::GestureData>& gestures = handTrackerFrame.getGestures();
	for (int i = 0; i < gestures.getSize(); ++i)
	{
		if (gestures[i].isComplete())
		{
			nite::HandId newId;
			handTracker.startHandTracking(gestures[i].getCurrentPosition(), &newId);
		}
	}

	const nite::Array<nite::HandData>& hands = handTrackerFrame.getHands();
	for (int i = 0; i < hands.getSize(); ++i)
	{
		const nite::HandData& hand = hands[i];
		if (hand.isTracking())
		{
			printf("%d. (%5.2f, %5.2f, %5.2f)\n", hand.getId(), hand.getPosition().x, hand.getPosition().y, hand.getPosition().z);
		}
	}
}

//nite::NiTE::shutdown();
*/