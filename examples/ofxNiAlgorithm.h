#pragma once

#include "ofxOpenNI2.h"
#include "ofxUI.h"


class ofxNiAlgorithm
{
public:

	void setup(const ofxDepthStream* depthStream, const ofxColorStream* colorStream);

	virtual void customSetup(){};
	virtual void update(){};
	virtual void draw(){};
	virtual void setupGui(){};

protected:
	const ofxDepthStream* depthStream;
	const ofxColorStream* colorStream;

	ofPtr<ofxUICanvas> gui;   	


};

