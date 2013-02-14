#pragma once

#include "ofxNiAlgorithm.h"


class ColorFingerTracker : public ofxNiAlgorithm
{
public:
	
	virtual void update();
	virtual void draw();

	virtual void setupGui();

	virtual void customSetup();

	bool isValid() const { return valid; }

protected:

	bool valid;
	ofPoint fingMean;
	ofPoint fingDir;

	ofxUISlider* satThreshold;
	ofxUISlider* valueThreshold;

};

