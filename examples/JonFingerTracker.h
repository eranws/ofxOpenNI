#pragma once

#include "opencv2/opencv.hpp"


class JonFingerTracker
{
public:
	JonFingerTracker(void);
	~JonFingerTracker(void);

	void detectFinger(cv::Mat& handFrame, const cv::Rect& handRect);

private:
	cv::Point2f g_hand_fingerPoint2f; // in handFrame coordinates
	cv::Point2f g_fingerPoint2f; //in frameCoordinates

	cv::Point2f g_handPoint2f, g_prevHandPoint;
	bool g_hasFinger;

};
