#pragma once

#include "ofMain.h"

#include "ofxOniDevice.h"
#include "ofxDepthStream.h"
#include "ofxColorStream.h"
#include "ofxOpenNi2.h"

#include <stdint.h>

#include "ofxUI.h"
#include "ofxUIRadio.h"


typedef map<unsigned long, int> ClickMap;


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
		ClickMap clickMap;
		
		RecordingGroundTruth()
		{
			clear();
		}
		void clear()
		{
			startTime = 0;
			gridWidth = 0;
			gridHeight = 0;
			clickMap.clear();
		}
	} truth;

	map<uint64_t, int> timestamps;

	ofPtr<ofxUICanvas> gui;   	
	void guiEvent(ofxUIEventArgs &e);
	
	void setupLeftGui(); 
	void setupBottomGui();
	void setupRightGui(); 


	virtual void mouseMoved( int x, int y );

	virtual void keyPressed( int key );
	void loadFile( string files );

	ofPtr<ofxUICanvas> bottomPanel;
	ofPtr<ofxUICanvas> rightPanel;
	ofPtr<ofxUICanvas> leftPanel;
	int nFrames; //mv to playbackcontrols
	
	string dirPath;


};

