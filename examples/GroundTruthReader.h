#pragma once

#include "ofMain.h"

#include "ofxOniDevice.h"
#include "ofxDepthStream.h"
#include "ofxColorStream.h"
#include "ofxOpenNi2.h"

class GroundTruthReader : public ofBaseApp
{
public:
	virtual void dragEvent( ofDragInfo dragInfo );

	virtual void draw();

	virtual void setup();


private:
	ofxOpenNi2 oni2;
	ofxOniDevice oniDevice;
	ofxDepthStream depthStream;
	ofxColorStream colorStream;

	struct RecordingGroundTruth
	{
		unsigned long startTime;
		int gridWidth;
		int gridHeight;
		map<unsigned long, int> selections;

		RecordingGroundTruth()
		{
			clear();
		}
		void clear()
		{
			startTime = 0;
			gridWidth = 0;
			gridHeight = 0;
			selections.clear();
		}
	} truth;

};

