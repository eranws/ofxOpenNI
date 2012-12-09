#ifndef _TEST_APP
#define _TEST_APP

#include "OpenNI.h"
#include "NiTE.h"


#include "ofMain.h"

#include "ofxUI\src\ofxUI.h"
#include <map>


#define MAX_DEVICES 2
#define MAX_HANDS 4


struct XHead
{
	nite::UserId id;
	ofVec3f pos;

	friend bool operator<(XHead& a, XHead& b);

};

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
	ofPixels* colorPixelsDoubleBuffer[2];

	ofShortPixels* userPixelsDoubleBuffer[2];

	ofTexture depthTexture;
	ofTexture colorTexture;
	
	ofTexture usersTexture;
	ofTexture frontUserColorTexture;


	void setGUI4(); 	    
	ofxUIScrollableCanvas *gui4;
	ofxUIMovingGraph *mg; 
	float buffer[256]; 
	float falafelSizeFactor;

	ofImage bgImage;
	//TODO: make a collection of these...
	ofImage item;
	ofVec2f itemSize;

	static const int handImageCount = 4;
	ofImage hand[handImageCount];
	ofPoint handPos;


	float red, green, blue; 


	nite::UserTrackerFrameRef userTrackerFrame;
	nite::UserTracker* userTracker;

	typedef std::vector<XHead> HeadMap;
	HeadMap* headMaps[2];


	bool _closing;
};

#endif
