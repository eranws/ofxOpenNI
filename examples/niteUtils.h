#include "NiTE.h"

#define MAX_USERS 10
nite::UserState g_userStates[MAX_USERS] = {(nite::UserState)-1};
nite::SkeletonState g_skeletonStates[MAX_USERS] = {nite::SKELETON_NONE};

#define USER_MESSAGE(msg) \
{printf("[%08llu] User #%d:\t%s\n",ts, user.getId(),msg); break;}

void printUserState(const nite::UserData& user, unsigned long long ts)
{
	if(g_userStates[user.getId()] != user.getState())
	{
		switch(g_userStates[user.getId()] = user.getState())
		{
		case nite::USER_STATE_NEW:
			USER_MESSAGE("New")
		case nite::USER_STATE_VISIBLE:
			USER_MESSAGE("Visible")
		case nite::USER_STATE_OUT_OF_SCENE:
			USER_MESSAGE("Out of scene...")
		case nite::USER_STATE_LOST:
			USER_MESSAGE("Lost.")
		}
	}
	if(g_skeletonStates[user.getId()] != user.getSkeleton().getState())
	{
		switch(g_skeletonStates[user.getId()] = user.getSkeleton().getState())
		{
		case nite::SKELETON_NONE:
			USER_MESSAGE("Stopped tracking.")
		case nite::SKELETON_CALIBRATING:
			USER_MESSAGE("Calibrating...")
		case nite::SKELETON_TRACKED:
			USER_MESSAGE("Tracking!")
		case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
		case nite::SKELETON_CALIBRATION_ERROR_HANDS:
		case nite::SKELETON_CALIBRATION_ERROR_LEGS:
		case nite::SKELETON_CALIBRATION_ERROR_HEAD:
		case nite::SKELETON_CALIBRATION_ERROR_TORSO:
			USER_MESSAGE("Calibration Failed... :-|")
		}
	}
}


bool debugPrintMiddlePixel = false;
if (debugPrintMiddlePixel) //TODO: move to Utils
{
	int middleIndex = (depthFrame.getHeight()+1)*depthFrame.getWidth()/2;
	DepthPixel* pDepth = (DepthPixel*)depthFrame.getData();
	printf("[%08llu] %8d\n", depthFrame.getTimestamp(), pDepth[middleIndex]);
}
