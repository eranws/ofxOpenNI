#include "ofxHandTracker.h"

#include "Nite.h"

void ofxHandTracker::setup(ofPtr<openni::Device> device, bool isVerbose)
{
	niteRc = nite::STATUS_OK;

	this->device = device;

	niteRc = handTracker.create(device.get());
	if (niteRc != nite::STATUS_OK)
	{
		throw ("Couldn't create user tracker\n");
	}

	handTracker.startGestureDetection(nite::GESTURE_WAVE);
	handTracker.startGestureDetection(nite::GESTURE_CLICK);
	handTracker.startGestureDetection(nite::GESTURE_HAND_RAISE);

	handTrackerFrame = ofPtr<nite::HandTrackerFrameRef>(new nite::HandTrackerFrameRef);
	
	frameIndex = -1;
//	startThread(false, isVerbose);
}

void ofxHandTracker::exit()
{
	stopThread();
	waitForThread();

	handTrackerFrame->release();
	handTracker.destroy();
}

void ofxHandTracker::threadedFunction()
{
	while (isThreadRunning())
	{
		readFrame();
	}

}

void ofxHandTracker::readFrame()
{
	niteRc = handTracker.readFrame(handTrackerFrame.get());
	if (niteRc != nite::STATUS_OK)
	{
		printf("Get next frame failed\n");
		return;
	}
	frameIndex = handTrackerFrame->getFrameIndex();
	timestamp = handTrackerFrame->getTimestamp();
	const nite::Array<nite::GestureData>& gestures = handTrackerFrame->getGestures();
	for (int i = 0; i < gestures.getSize(); ++i)
	{
		if (gestures[i].isComplete())
		{
			nite::HandId newId;
			handTracker.startHandTracking(gestures[i].getCurrentPosition(), &newId);
		}
	}

	const nite::Array<nite::HandData>& hands = handTrackerFrame->getHands();
	for (int i = 0; i < hands.getSize(); ++i)
	{
		const nite::HandData& hand = hands[i];
		if (hand.isTracking())
		{
			handPoint = ofPoint(hand.getPosition().x, hand.getPosition().y, hand.getPosition().z);
			//printf("%d. (%5.2f, %5.2f, %5.2f)\n", hand.getId(), hand.getPosition().x, hand.getPosition().y, hand.getPosition().z);
		}
	}

}



