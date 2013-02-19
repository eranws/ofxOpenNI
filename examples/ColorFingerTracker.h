#pragma once

#include "ofxNiAlgorithm.h"
#include "opencv2\opencv.hpp"


class ofJoint
{
public:
	ofJoint(){valid = false;}

	bool isValid() const { return valid; }
	void setValid(bool val)
	{
		valid = val;
		if (!val)
		{
			prev = ofPoint();
		}
	}

	ofPoint getPos() const { return pos; }
	void setPos(ofPoint val)
	{
		if (prev == ofPoint()) prev = val;
		const float a = 0.7;
		pos = a * val + (1-a) * prev;
		prev = pos;
	}

	operator bool(){return valid;}
	operator ofPoint(){return pos;}


private:
	bool valid;
	ofPoint pos;

	ofPoint prev;
};

class ColorFingerTracker : public ofxNiAlgorithm
{
public:
	
	virtual void update();
	virtual void draw();

	virtual void setupGui();

	ofJoint getFingerTip() const { return fingerTip; }
	ofJoint getFingerBase() const { return fingerBase; }


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

