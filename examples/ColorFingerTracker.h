#pragma once

#include "ofxNiAlgorithm.h"
#include "opencv2\opencv.hpp"


class ofJoint
{
public:
	ofJoint(){valid = false;}

	bool isValid() const { return valid; }
	ofPoint getPos() const { return pos; }
	operator ofPoint(){return pos;}

private:
	bool valid;
	ofPoint pos;

	friend class ColorFingerTracker;
};

class ColorFingerTracker : public ofxNiAlgorithm
{
public:
	
	virtual void update();
	virtual void draw();

	virtual void setupGui();

protected:


	void detectWrist( cv::Mat greenMask, const cv::Mat depthMat );
	void detectKnuckle( cv::Mat knuckleMask, const cv::Mat depthMat );
	void detectFinger(const cv::Mat& fingerMask, const cv::Mat& depthMat);
	vector<ofPoint> getContourRealPoints( std::vector<cv::Point> biggestContour, const cv::Mat& depthMat, int medianZ, int param3 );

	ofJoint fingerTip;
	ofJoint fingerBase;		//end of the red region
	ofJoint fingerKnuckle;	//yellow region
	ofJoint wrist;
	ofJoint shoulder;

	
	ofxUIRangeSlider* satRange;
	ofxUIRangeSlider* valRange;

	ofxUISlider* redSlider;
	ofxUISlider* yellowSlider;
	ofxUISlider* greenSlider;

	ofxUISlider* hueRange;
};

