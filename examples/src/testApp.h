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

	ofImage item;
	ofVec2f itemSize;

	static const int handImageCount = 4;
	ofImage hand[handImageCount];
	ofVec2f handPos;
	ofVec2f handSize;
	float handSizeFactor;


	ofImage text;
	ofVec2f textPos;
	ofVec2f textSize;
	float textSizeFactor;
	ofVec2f textAnimationTarget;
	ofVec2f textAnimation;


	static const int instImageCount = 9;
	ofImage inst[instImageCount];
	ofVec2f instPos;
	ofVec2f instSize;
	float instSizeFactor;

	ofImage crown;
	ofVec2f crownPos;
	ofVec2f crownSize;
	float crownSizeFactor;

	static const int bgImageCount = 2;
	ofImage bgImage[bgImageCount];
	ofVec2f bgSize;
	float bgSizeFactor;


	bool drawColorBackground;
	float bgProgress;
	bool animateBg;





	float red, green, blue; 


	nite::UserTrackerFrameRef userTrackerFrame;
	nite::UserTracker* userTracker;

	typedef std::vector<XHead> HeadMap;
	HeadMap* headMaps[2];


	bool _closing;
};

#endif
