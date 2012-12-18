#include "ofxNite2.h"



ofxNite2::ofxNite2(void)
{
}


ofxNite2::~ofxNite2(void)
{
	//wait for thread to exit
	nite::NiTE::shutdown();
}

int ofxNite2::setup()
{
	nite::Status niteRc;

	niteRc = nite::NiTE::initialize();
	if (niteRc != nite::STATUS_OK)
	{
		printf("NiTE initialization failed\n");
		return 1;
	}

	niteRc = handTracker.create();
	if (niteRc != nite::STATUS_OK)
	{
		printf("Couldn't create user tracker\n");
		return 3;
	}

	handTracker.startGestureDetection(nite::GESTURE_WAVE);
	handTracker.startGestureDetection(nite::GESTURE_CLICK);
	printf("\nWave or click to start tracking your hand...\n");

}


void ofxNite2::update()
{
	nite::HandTrackerFrameRef handTrackerFrame;
	nite::Status niteRc;
	niteRc = handTracker.readFrame(&handTrackerFrame);
	if (niteRc != nite::STATUS_OK)
	{
		printf("Get next frame failed\n");
		return;
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