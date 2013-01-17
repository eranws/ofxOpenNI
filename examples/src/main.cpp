
#include "testApp.h"
#include "ofMain.h"
#include "ofAppGlutWindow.h"
#include "..\GroundTruthReader.h"

//========================================================================
int main( ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 640*2,480*2, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
//	ofRunApp( new GroundTruthReader());
	ofRunApp( new testApp());


}
