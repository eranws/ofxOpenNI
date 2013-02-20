#include "ofxNiAlgorithm.h"


//TODO: replace with ofxOniDevice
void ofxNiAlgorithm::setup(const ofxDepthStream* depthStream, const ofxColorStream* colorStream)
{
	this->depthStream = depthStream;
	this->colorStream = colorStream;
	customSetup();


	setupGui();
	//TODO: register update callbacks
}


void ofxNiAlgorithm::setupGui()
{
	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float length = 255-xInit; 
	gui = ofPtr<ofxUICanvas>(new ofxUICanvas(0, 0, length+xInit, ofGetHeight())); 
	customSetupGui();
}
