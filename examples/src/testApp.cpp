#include "testApp.h"
#include <string>
#include "ofxCv\Utilities.h"

//--------------------------------------------------------------
void testApp::setup() {

	ofSetLogLevel(OF_LOG_VERBOSE);

	openNIDevice.setup();
	//openNIDevice.setupFromONI("C:/f/q.oni");

	openNIDevice.addImageGenerator();
	openNIDevice.addDepthGenerator();
	openNIDevice.setRegister(true);
	openNIDevice.setMirror(true);

	// setup the hand generator
	openNIDevice.addHandsGenerator();

	// add all focus gestures (ie., wave, click, raise arm)
	openNIDevice.addAllHandFocusGestures();

	// or you can add them one at a time
	//vector<string> gestureNames = openNIDevice.getAvailableGestures(); // you can use this to get a list of gestures
	// prints to console and/or you can use the returned vector
	//openNIDevice.addHandFocusGesture("Wave");

	openNIDevice.setMaxNumHands(4);

	for(int i = 0; i < openNIDevice.getMaxNumHands(); i++){
		ofxOpenNIDepthThreshold depthThreshold = ofxOpenNIDepthThreshold(0, 0, false, true, true, true, true); 
		// ofxOpenNIDepthThreshold is overloaded, has defaults and can take a lot of different parameters, eg:
		// (ofxOpenNIROI OR) int _nearThreshold, int _farThreshold, bool _bUsePointCloud = false, bool _bUseMaskPixels = true, 
		// bool _bUseMaskTexture = true, bool _bUseDepthPixels = false, bool _bUseDepthTexture = false, 
		// int _pointCloudDrawSize = 2, int _pointCloudResolution = 2
		depthThreshold.setUsePointCloud(true);
		depthThreshold.setPointCloudDrawSize(2);

		openNIDevice.addDepthThreshold(depthThreshold);

	}

	openNIDevice.start();

	cam.setDistance(10);
	faceTracker.setup();

	verdana.loadFont(ofToDataPath("verdana.ttf"), 24);
}

//--------------------------------------------------------------
void testApp::update(){
	openNIDevice.update();

	if(openNIDevice.isNewFrame()) {
		faceTracker.update(ofxCv::toCv(openNIDevice.getImagePixels()));
	}

	// reset all depthThresholds to 0,0,0
	for(int i = 0; i < openNIDevice.getMaxNumHands(); i++){
		ofxOpenNIDepthThreshold & depthThreshold = openNIDevice.getDepthThreshold(i);
		ofPoint leftBottomNearWorld = ofPoint(0,0,0);
		ofPoint rightTopFarWorld = ofPoint(0,0,0);
		ofxOpenNIROI roi = ofxOpenNIROI(leftBottomNearWorld, rightTopFarWorld);
		depthThreshold.setROI(roi);
	}

	// iterate through users
	for (int i = 0; i < openNIDevice.getNumTrackedHands(); i++){

		// get a reference to this user
		ofxOpenNIHand & hand = openNIDevice.getTrackedHand(i);

		// get hand position
		ofPoint & handWorldPosition = hand.getWorldPosition(); // remember to use world position for setting ROIs!!!

		// set depthThresholds based on handPosition
		ofxOpenNIDepthThreshold & depthThreshold = openNIDevice.getDepthThreshold(i); // we just use hand index for the depth threshold index
		ofPoint leftBottomNearWorld = handWorldPosition - 100; // ofPoint has operator overloading so it'll subtract/add 50 to x, y, z
		ofPoint rightTopFarWorld = handWorldPosition + 100;

		ofxOpenNIROI roi = ofxOpenNIROI(leftBottomNearWorld, rightTopFarWorld);
		depthThreshold.setROI(roi);

	}
}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(255, 255, 255);

	ofPushMatrix();
	// draw debug (ie., image, depth, skeleton)
	openNIDevice.drawDebug();
	ofPopMatrix();

	ofPushMatrix();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	if(faceTracker.getFound()) {
		ofxCv::applyMatrix(faceTracker.getRotationMatrix());
		ofScale(5,5,5);
		faceTracker.getObjectMesh().drawWireframe();
		ofDrawBitmapString(ofToString(faceTracker.getPosition()), 10, 20);
	}

	// iterate through users
	for (int i = 0; i < openNIDevice.getNumTrackedHands(); i++){

		// get a reference to this user
		ofxOpenNIHand & hand = openNIDevice.getTrackedHand(i);

		// get hand position
		ofPoint & handPosition = hand.getPosition();

		// draw a rect at the position
		ofSetColor(255,0,0);
		ofRect(handPosition.x, handPosition.y, 2, 2);
		ofDrawBitmapString("hand" + ofToString(i), handPosition.x, handPosition.y);
		// set depthThresholds based on handPosition
		ofxOpenNIDepthThreshold & depthThreshold = openNIDevice.getDepthThreshold(i); // we just use hand index for the depth threshold index
		depthThreshold.drawROI();

		
		ofMesh& pc = depthThreshold.getPointCloud();

		vector<ofVec3f> v = pc.getVertices();

		if (v.size() > 0)
		{
			ofVec3f minV = v[0];
			for (int iv=1; iv < v.size(); iv++)
			{
				if (v[iv].y - v[iv].z > minV.y - minV.z ) minV = v[iv];
			}

			// draw ROI over the depth image
			ofSetColor(255,255,255);
			cam.setGlobalPosition(0,0,handPosition.z + 400);
			cam.begin();

			//cam.lookAt(handPosition);//, ofVec3f(0, -1, 0));

			depthThreshold.getPointCloud().disableColors();
			depthThreshold.drawPointCloud();

			ofSetColor(ofColor::red, 128);
			ofSphere(minV, 5);

			cam.end();
		}
		// draw depth and mask textures below the depth image at 0.5 scale
		// you could instead just do pixel maths here for finger tracking etc
		// by using depthThreshold.getDepthPixels() and/or depthThreshold.getMaskPixels()
		// and turn off the textures in the initial setup/addDepthTexture calls

		ofPushMatrix();
		ofTranslate(320 * i, 480);
		ofScale(0.5, 0.5);
		depthThreshold.drawDepth();
		depthThreshold.drawMask();
		ofPopMatrix();

		// i think this is pretty good but might be a frame behind???

	}

	ofDisableBlendMode();
	ofPopMatrix();

	// draw some info regarding frame counts etc
	ofSetColor(0, 255, 0);
	string msg = " MILLIS: " + ofToString(ofGetElapsedTimeMillis()) + " FPS: " + ofToString(ofGetFrameRate()) + " Device FPS: " + ofToString(openNIDevice.getFrameRate());

	verdana.drawString(msg, 20, 480 - 20);
}

//--------------------------------------------------------------
void testApp::handEvent(ofxOpenNIHandEvent & event){
	// show hand event messages in the console
	ofLogNotice() << getHandStatusAsString(event.handStatus) << "for hand" << event.id << "from device" << event.deviceID;
}

//--------------------------------------------------------------
void testApp::exit(){

	faceTracker.stopThread();
	faceTracker.waitForThread();

	openNIDevice.stop();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}