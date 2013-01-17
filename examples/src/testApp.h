#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxFaceTrackerThreaded.h"
#include "..\Scene.h"
#include "..\ofxHandTracker.h"
#include "..\ofxDepthStream.h"
#include "..\ofxOniDevice.h"
#include "..\ofxColorStream.h"
#include "..\Keypad.h"
#include "..\ofxRecorder.h"
#include "ofxUI.h"
#include "..\ofxOpenNi2.h"
#include "..\ofxNite2.h"

#define MAX_DEVICES 2
#define MAX_HANDS 4

class testApp : public ofBaseApp
{
	static const string MODULE_NAME;

public:

	void setup();
	void update();
	void draw();
	void exit();

	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	void handUpdate(ofPoint& p);

private:

//	void handEvent(ofxOpenNIHandEvent & event);
	ofxOpenNi2 oni2;
	ofxNite2 nite2;

	ofxOniDevice oniDevice;
	ofxDepthStream depthStream;
	ofxColorStream colorStream;
	ofxHandTracker handTracker;
	ofxRecorder recorder;

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
	Scene scene;


	bool showProfilerString;
	bool drawDebugString;
	bool drawOpenNiDebug;

	stringstream debugString;

	ofVec3f facePos;
	ofVec2f screenPoint;
	deque<ofVec2f> screenPointHistory;

	Keypad keypad;

	ofFile file;

	ofPtr<ofxUICanvas> gui1;   	
	void setupGui(); 
	void guiEvent(ofxUIEventArgs &e);

	ofxUIToggle* faceToggle; 

	deque<cv::Mat> depthHistory;
	int depthHistorySize;

	IplImage* motion;

};

#endif
