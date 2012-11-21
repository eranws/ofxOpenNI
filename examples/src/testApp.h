#ifndef _TEST_APP
#define _TEST_APP

#include "OpenNI.h"
#include "NiTE.h"


#include "ofMain.h"
#include "ofxFaceTrackerThreaded.h"

#include "..\..\..\git\github\ofxVirtualScreen\ofxVirtualScreen.h"

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
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	void guiEvent(ofxUIEventArgs &e);

	virtual void onNewFrame( openni::VideoStream& stream);

	virtual void onNewFrame();



private:

//	void handEvent(ofxOpenNIHandEvent & event);
	int setupOpenNi();
	int setupNite();
	int start();



	ofxFaceTrackerThreaded faceTracker;

	ofEasyCam handCam;
	ofEasyCam sceneCam;


	struct Finger
	{
		std::deque<ofVec3f> position;
		bool isTracked;
		Finger() : isTracked(false){ position.push_front(ofVec3f());}
		ofVec3f getFilteredPosition(float a = 0.5f);

		static const int historySize = 10;
	};

	Finger fingers[MAX_HANDS];

	bool drawDebugString;
	bool drawOpenNiDebug;

	stringstream camString;

	ofVec3f facePos;
	ofVec2f screenPoint;
	deque<ofVec2f> screenPointHistory;

private:
	openni::VideoFrameRef frame;

	openni::VideoStream depthStream;
	openni::Device device;
	
	ofShortPixels* depthPixelsDoubleBuffer[2];

	ofPixels colorPixels;

	ofTexture depthTexture;

	ofTexture texScreen;
	int counter;


	Scene scene;

	void setGUI4(); 	    
	ofxUIScrollableCanvas *gui4;
	ofxUIMovingGraph *mg; 
	float *buffer; 
	ofImage *img; 

	ofImage bgImage;

	//set of these...
	ofImage item;
	ofPoint itemPos;
	ofVec2f itemSize;
	float itemSizeFactor; //according to head distance

	float red, green, blue; 
	bool bdrawGrid; 
	bool bdrawPadding; 

	deque<nite::UserTrackerFrameRef> userTrackerFrameDeque;
	nite::UserTrackerFrameRef userTrackerFrame;
	nite::UserTracker userTracker;



};

#endif
