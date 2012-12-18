#pragma once

#include "NiTE.h"

class ofxNite2
{
public:
	ofxNite2(void);
	~ofxNite2(void);
	int setup();
	void update();

	nite::HandTracker handTracker;

};
