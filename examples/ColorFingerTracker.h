#pragma once

#include "ofxNiAlgorithm.h"


class ColorFingerTracker : public ofxNiAlgorithm
{
public:
	virtual void update();
	virtual void draw();

	virtual void setupGui();

protected:
	ofPoint fingMean;
	ofPoint fingDir;

	ofxUISlider* satThreshold;
	ofxUISlider* valueThreshold;

};

