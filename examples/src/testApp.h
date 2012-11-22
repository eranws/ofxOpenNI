#ifndef _TEST_APP
#define _TEST_APP

#include "OpenNI.h"
#include "NiTE.h"


#include "ofMain.h"

#include "ofxUI\src\ofxUI.h"


#define MAX_DEVICES 2
#define MAX_HANDS 4

class testApp : public ofBaseApp, public openni::VideoStream::Listener, public openni::OpenNI::Listener, nite::UserTracker::Listener
{

public:

	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed(int key);
	void mouseMoved(int x, int y);
	void windowResized(int w, int h);

	void guiEvent(ofxUIEventArgs &e);

	virtual void onNewFrame( openni::VideoStream& stream);

private:
	int setupOpenNi();
	int setupNite();
	int start();

	bool drawDebug;
	bool drawDebugString;
	bool drawOpenNiDebug;

	stringstream debugString;

	openni::VideoFrameRef colorFrame;
	openni::VideoFrameRef depthFrame;

	openni::VideoStream colorStream;
	openni::VideoStream depthStream;
	openni::Device device;

	
	ofShortPixels* depthPixelsDoubleBuffer[2];
	ofImage sensorImage;


	ofTexture depthTexture;

	void setGUI4(); 	    
	ofxUIScrollableCanvas *gui4;
	ofxUIMovingGraph *mg; 
	float buffer[256]; 

	ofImage bgImage;
	//TODO: make a collection of these...
	ofImage item;
	ofPoint itemPos;
	ofVec2f itemSize;
	float itemSizeFactor; //according to head distance

	float red, green, blue; 

	ofVec2f headScreenPos;

	nite::UserTrackerFrameRef userTrackerFrame;
	nite::UserTracker userTracker;

};

#endif
