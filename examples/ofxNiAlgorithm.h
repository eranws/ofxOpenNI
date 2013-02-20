#pragma once

#include "ofxOpenNI2.h"
#include "ofxUI.h"


class ofxNiAlgorithm
{
public:
	void setup(const ofxDepthStream* depthStream, const ofxColorStream* colorStream);

protected:
	
	virtual void customSetupGui(){};
	virtual void customSetup(){};
	virtual void update(){};
	virtual void draw(){};
	
	const ofxDepthStream* depthStream;
	const ofxColorStream* colorStream;

	ofPtr<ofxUICanvas> gui;   	

private:
	void setupGui();



};

