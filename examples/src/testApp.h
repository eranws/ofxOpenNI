#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofxFaceTrackerThreaded.h"
#include "..\Scene.h"
#include "..\ofxHandTracker.h"

#include "..\Keypad.h"
#include "ofxUI.h"
#include "..\ofxOpenNi2.h"
#include "..\ofxNite2.h"
#include "ofxToast\src\ofxToast.h"
#include "..\ColorFingerTracker.h"

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

	cv::Mat depthMat;

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

	ofEasyCam sceneCam;
	
	ofxToast toast;

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

	stringstream debugString;
	stringstream logString;

	ofVec3f facePos;
	ofVec2f screenPoint;
	deque<ofVec2f> screenPointHistory;

	Keypad keypad;

	ofFile file;

	ofPtr<ofxUICanvas> gui1;   	
	void setupGui(); 
	void guiEvent(ofxUIEventArgs &e);
	void dumpGroundTruth();
	ofxUIToggle* guiAutoHide;
	ofxUIToggle* playToggle;
	ofxUIToggle* faceToggle; 
	ofxUIToggle* cvDepthToggle;
	ofxUIToggle* fullScreenToggle;

	ofxUIToggle* depthThresholding;
	ofxUIToggle* velocityMasking;
	ofxUIToggle* computeHistory;

	ofxUIToggle* detectFingerToggle;

	ofxUIToggle* drawHand;
	ofxUIToggle* drawHandHistory;
	ofxUIToggle* drawFingerHistory;

	ofxUIMovingGraph* mgZ; 
	ofxUIMovingGraph* mgA;
	ofxUIMovingGraph* mgApca;
	ofxUIMovingGraph* mgB; 
	ofxUIMovingGraph* mgC; 
	ofxUIMovingGraph* mgErr;
	ofxUIMovingGraph* mgErrPca;


	deque<ofPoint> fingerHistory;
	static const int fingerHistorySize = 7;
	ofPoint fingerHistoryScreenIntersectionPoint;

	deque<ofPoint> fingerWristHistory;
	static const int fingerWristHistorySize = 7;
	cv::Mat AA; // [(At * A) ^(-1)] * At
	cv::Mat A;
	ofxUISlider* aThreshold;
	ofxUISlider* errThreshold;

	ofxUISlider* apcaThreshold;
	ofxUISlider* errpcaThreshold;

	int lastClicked;

	int frameIndex;
	bool toUpdate;
	bool readOnUpdate;
	bool logToFile;

	vector<string> clicks;
	
	int recordingStartFrame;
	string recordingFilename;

	struct Joint
	{
		ofPoint pos;
		int frame;
		
		operator ofPoint(){return pos;}
	};

	Joint wrist;
	Joint hand;
	Joint finger;


	std::deque<ofPoint> handHistory;
	int handHistorySize;

	deque<cv::Mat> depthHistory;
	int depthHistorySize;

	std::map<const char*, cv::Mat> matMap;

	ColorFingerTracker colorFingerTracker;
	
};

#endif
