#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"

#include "ofxUI\src\ofxUI.h"


#define MAX_DEVICES 2
#define MAX_HANDS 4

class testApp : public ofBaseApp
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




private:

	void setGUI4(); 	    

	stringstream camString;

	ofVec3f facePos;
	ofVec2f screenPoint;
	deque<ofVec2f> screenPointHistory;



	ofPixels colorPixels;

	ofxUIScrollableCanvas *gui4;
	ofxUIMovingGraph *mg; 
	float buffer[256]; 

	ofImage bgImage;

	//collection of these...
	ofImage item;
	ofPoint itemPos;
	ofVec2f itemSize;
	float itemSizeFactor; //according to head distance

	float red, green, blue; 
	bool bdrawGrid; 
	bool bdrawPadding; 

};

#endif
