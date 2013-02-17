#pragma once

#include "ofxNiAlgorithm.h"


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

	ofJoint fingerTip;
	ofJoint fingerBase;
	ofJoint wrist;
	ofJoint shoulder;

	
	ofxUIRangeSlider* satRange;
	ofxUIRangeSlider* valRange;

	ofxUISlider* redSlider;
	ofxUISlider* yellowSlider;
	ofxUISlider* greenSlider;

	ofxUISlider* hueRange;
};

