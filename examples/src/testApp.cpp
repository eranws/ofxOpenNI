#include "testApp.h"
#include "ofxCv\Utilities.h"

#define PROFILE
#ifdef PROFILE
#include "src\ofxProfile.h"
#endif
#include <math.h>

//--------------------------------------------------------------
void testApp::setup() {

	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetFrameRate(100);

	try
	{
		oniDevice.setup();
	}
	catch (exception e)
	{
		oniDevice.setup("c:\\1.oni");
	}
	

	depthStream.setup(oniDevice.getDevice());
	colorStream.setup(oniDevice.getDevice());
	oniDevice.setRegistration(true);

	handTracker.setup(depthStream.getDevice());

	handCam.setDistance(10);
	faceTracker.setup();

	sceneCam.setGlobalPosition(0,0,2000);

}

//--------------------------------------------------------------
void testApp::update(){
	ofxProfileThisFunction();
	
	debugString = stringstream();
	
	
	//if(depthStream.isNewFrame()) {
		ofxProfileSectionPush("faceTracker update");

		ofxProfileSectionPush("ofPixels ofPixels = openNIDevice.getImagePixels();");
		ofPixels ofPixels = colorStream.getPixels();
		ofxProfileSectionPop();


		ofxProfileSectionPush("cv::Mat mat = ofxCv::toCv(ofPixels);");
		cv::Mat mat = ofxCv::toCv(ofPixels);
		ofxProfileSectionPop();

		ofxProfileSectionPush("faceTracker.update(mat);");
		faceTracker.update(mat);
		ofxProfileSectionPop();
		
		ofxProfileSectionPop();

		if(!faceTracker.getFound())
		{
			facePos = ofVec3f();
			screenPoint = ofVec2f();
		}


#ifdef OPENNI1
	// reset all depthThresholds to 0,0,0
	for(int i = 0; i < openNIDevice.getMaxNumHands(); i++){
		ofxOpenNIDepthThreshold & depthThreshold = openNIDevice.getDepthThreshold(i);
		ofPoint leftBottomNearWorld = ofPoint(0,0,0);
		ofPoint rightTopFarWorld = ofPoint(0,0,0);
		ofxOpenNIROI roi = ofxOpenNIROI(leftBottomNearWorld, rightTopFarWorld);
		depthThreshold.setROI(roi);
	}

	for (int i = 0; i < MAX_HANDS; i++)
	{
		fingers[i].isTracked = false;
	}
	// iterate through users

	ofxProfileSectionPush("iterate hands");
	for (int i = 0; i < openNIDevice.getNumTrackedHands(); i++)
	{
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


		openNIDevice.lock();
		ofMesh& pc = depthThreshold.getPointCloud();
		vector<ofVec3f> vRef = pc.getVertices();
		vector<ofVec3f> v(vRef.begin(), vRef.end());
		openNIDevice.unlock();

		if (v.size() > 0)
		{
			fingers[i].isTracked = true;
		}

		//TODO: export to finger tracker module
		if (fingers[i].isTracked)
		{
			ofVec3f minV = v[0];
			for (int iv=1; iv < v.size(); iv++)
			{
				// closest topmost
				if (v[iv].y - v[iv].z > minV.y - minV.z) minV = v[iv];
			}

			//get finger center of mass
			ofVec3f fingerCoM;
			int count = 0;
			for (int iv=0; iv < v.size(); iv++)
			{
				
				if (minV.distanceSquared(v[iv]) < 100)
				{
					fingerCoM += v[iv];
					count++;
				}

			}
			fingerCoM /= count;
			debugString << "count" << count;


			fingers[i].position.push_front(fingerCoM);
			if (fingers[i].position.size() > Finger::historySize)
			{
				fingers[i].position.pop_back();
			}


		}
		else
		{
			fingers[i].position.clear();
		}

	}
#endif

	ofxProfileSectionPop();

}

//--------------------------------------------------------------
void testApp::draw(){
	ofxProfileThisFunction();
	ofBackground(0);


	if (drawOpenNiDebug)
	{
		ofxProfileSectionPush("draw OpenNiDebug");

		ofPushMatrix();
		ofPushStyle();

#ifdef OPENNI1
		openNIDevice.drawDebug(); // draw debug (ie., image, depth, skeleton)
#endif
		ofPopMatrix();
		ofxProfileSectionPop();
	}

	ofSetColor(255);

	ofTexture colorTexture;
	ofPixels colorPixels = colorStream.getPixels(); 
	colorTexture.allocate(colorPixels);
	colorTexture.loadData(colorPixels);
	colorTexture.draw(0,0);

	ofTexture depthTexture;
	ofShortPixels depthRawPixels = depthStream.getPixels();
	
	ofPixels depthPixels;
	depthPixels.allocate(depthRawPixels.getWidth(), depthRawPixels.getHeight(), OF_PIXELS_RGBA);
	
	const unsigned short* prd = depthRawPixels.getPixels();
	unsigned char* pd = depthPixels.getPixels();
	for (int i = 0; i < depthRawPixels.size(); i++)
	{
		const short minDepth = 450;
		short s = prd[i];
		char x = (s < minDepth) ? 0 : powf(s - minDepth, 0.7f);
		pd[4 * i + 0] = 255 - x;
		pd[4 * i + 1] = 255 - x;
		pd[4 * i + 2] = 255 - x;
		pd[4 * i + 3] = x;

	}


	depthTexture.allocate(depthPixels);
	depthTexture.loadData(depthPixels);
	depthTexture.draw(640,0);

	colorTexture.draw(320,0);
	depthTexture.draw(320,0);


	sceneCam.begin();
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	scene.draw();

	ofSetColor(255);
	colorTexture.draw(0,0);


	/*
	//////////////////////////////////////////////////////////////////////////
	// TODO move to history Filter...
	std::deque<ofPoint> points = handTracker.positionHistory();
	for (int i = 0; i < points.size(); i++)
	{
		ofSphere(points[i], 10);
	}

	if (points.size() == handTracker.historySize())
	{
		for (int i = 1; i < points.size(); i++)
		{
			ofSetLineWidth(points.size() - i);
			ofLine(points[i-1], points[i]);
		}
	}
	*/




#define camlog(func) {debugString << #func << " : " << sceneCam.func() << endl;}
	camlog(getDistance);
	camlog(getPosition);
	camlog(getOrientationEuler);
	camlog(getFarClip);
#undef camlog



	ofPushMatrix();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	
	if(faceTracker.getFound()) {

		ofPushMatrix();

		const float b = (facePos==ofVec3f()) ? 0 : 0.5;
		facePos = (b*facePos) + (1-b) * depthStream.cameraToWorld(faceTracker.getPosition());

		ofSetColor(ofColor::green);
		ofSphere(facePos, 5);

		ofTranslate(facePos);
		debugString << "facePos" << facePos;

		ofxCv::applyMatrix(faceTracker.getRotationMatrix());
		ofRotateY(180.0);
		faceTracker.getObjectMesh().drawWireframe();

		ofPopMatrix();
	}

#ifdef OPENNI1

	// iterate through users
	int oldestHandIndex = 0;
	for (int h = 0; h < openNIDevice.getNumTrackedHands(); h++)
	{
		 if (openNIDevice.getTrackedHand(h).getBirthTime() < openNIDevice.getTrackedHand(oldestHandIndex).getBirthTime())
			 oldestHandIndex = h;
	}

	int i=oldestHandIndex;
	if(openNIDevice.getNumTrackedHands() > 0)
	{
		// get a reference to this user
		ofxOpenNIHand & hand = openNIDevice.getTrackedHand(i);

		// get hand position
		ofPoint & handPosition = hand.getPosition();

		// draw a rect at the position
		ofSetColor(255,0,0);
		//ofRect(handPosition.x, handPosition.y, 2, 2);

		// set depthThresholds based on handPosition
		ofxOpenNIDepthThreshold & depthThreshold = openNIDevice.getDepthThreshold(i); // we just use hand index for the depth threshold index
		//depthThreshold.drawROI();


		if (fingers[i].isTracked)
		{
			// draw ROI over the depth image
			ofSetColor(255,255,255);
			//handCam.setGlobalPosition(0,0,handPosition.z + 400);
			//handCam.begin();
			//cam.lookAt(handPosition);//, ofVec3f(0, -1, 0));
			depthThreshold.getPointCloud().disableColors();
			depthThreshold.drawPointCloud();
			
			ofNoFill();
			

			ofSetColor(ofColor::blue, 128);
			ofSphere(fingers[i].getFilteredPosition(0.5), 5);

			//handCam.end();


			if(faceTracker.getFound())
			{
				ofPushStyle();
				ofSetLineWidth(3);
				ofSetColor(ofColor::green);
				ofDrawArrow(facePos, fingers[i].getFilteredPosition());
				ofSetLineWidth(1);
				ofSetColor(ofColor::yellow);
				ofLine(facePos, fingers[i].getFilteredPosition().interpolated(facePos, -3));
				ofPopStyle();

				ofSetColor(ofColor::magenta);

				ofxProfileSectionPush("getIntersectionPointWithLine");
				ofPoint screenIntersectionPoint = scene.screen.getIntersectionPointWithLine(facePos, fingers[i].getFilteredPosition());
				ofxProfileSectionPop();

				ofSphere(screenIntersectionPoint, 10);

				const float b = (screenPoint==ofVec2f()) ? 0 : 0.5;
				screenPoint = (b*screenPoint) + (1-b) * scene.screen.getScreenPointFromWorld(screenIntersectionPoint);
				
				screenPointHistory.push_front(screenPoint);
				if (screenPointHistory.size() > 10)
				{
					screenPointHistory.pop_back();
				}

			}

		}

	}
#endif

	ofDisableBlendMode();
	ofPopMatrix();

	sceneCam.end();

	if(faceTracker.getFound())
	{
		ofFill();
		ofSetColor(255);
		ofCircle(screenPoint, 10);
		ofNoFill();
		for (int i=0; i<screenPointHistory.size();i++)
		{
			int ri = screenPointHistory.size() - i - 1;
			ofSetColor(255,255,255,1 - 0.1*i);
			ofCircle(screenPointHistory[ri], 11 - ri);
		}
		debugString << "screenPoint: " << screenPoint << endl;
	}


	static float timeThen = 0;
	static float timeNow = 0;
	static float frameRate = 0;

	timeNow = ofGetElapsedTimef();
	double diff = timeNow-timeThen;	
	if( diff  > 0.00001 ){
		float fps = 1.0 / diff;
		frameRate	*= 0.9f;
		frameRate	+= 0.1f*fps;
	}
	timeThen = timeNow;

	// draw some info regarding frame counts etc
	ofSetColor(0, 255, 0);
	debugString << " Time: " << ofToString(ofGetElapsedTimeMillis() / 1000) << "." << ofToString(ofGetElapsedTimeMillis() % 1000) << endl;
	debugString << "frameRate: " << frameRate << endl;
	debugString << "fps: " << ofGetFrameRate() << endl;
//	camString << "Device FPS: " << openNIDevice.getFrameRate()<< endl;

	

#ifdef OPENNI1
	debugString << ofGetFrameRate() << endl << "Device FPS: " << openNIDevice.getFrameRate()<< endl;
#endif

	if (showProfilerString)
	{
		debugString << ofxProfile::describe();
	}

	if (drawDebugString)
	{
		ofSetColor(ofColor::green);
		ofDrawBitmapString(debugString.str(), 10, 20);
		//verdana.drawString(msg, 20, 480 - 20);
	}

}

#ifdef OPENNI1
//--------------------------------------------------------------
void testApp::handEvent(ofxOpenNIHandEvent & event){
	// show hand event messages in the console
	ofLogNotice() << getHandStatusAsString(event.handStatus) << "for hand" << event.id << "from device" << event.deviceID;
}
#endif

//--------------------------------------------------------------
void testApp::exit(){

	faceTracker.stopThread();
	faceTracker.waitForThread();

	handTracker.exit();
	colorStream.exit();
	depthStream.exit();
	oniDevice.exit();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	switch (key)
	{
	case '1': drawDebugString = !drawDebugString; break;
	case '2': drawOpenNiDebug = !drawOpenNiDebug; break;
	case '3': showProfilerString = !showProfilerString; break;

	case 'C': ofxProfile::clear(); break;

	case 'f': ofToggleFullscreen(); break;
	default:
		break;
	}

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

#ifdef OPENNI1

void testApp::setupOpenNiDevice()
{
	openNIDevice.setup();
	//openNIDevice.setupFromONI("C:/f/q.oni");

	openNIDevice.addImageGenerator();
	openNIDevice.addDepthGenerator();
	openNIDevice.setUseDepthRawPixels(true);
	openNIDevice.setUseBackBuffer(false);

	openNIDevice.setRegister(true);
	openNIDevice.setMirror(true);

	// setup the hand generator
	openNIDevice.addHandsGenerator();

	// add all focus gestures (ie., wave, click, raise arm)
	depthStream.addAllHandFocusGestures();

	// or you can add them one at a time
	//vector<string> gestureNames = openNIDevice.getAvailableGestures(); // you can use this to get a list of gestures
	// prints to console and/or you can use the returned vector
	//openNIDevice.addHandFocusGesture("Wave");

	openNIDevice.setMaxNumHands(MAX_HANDS);

	for(int i = 0; i < openNIDevice.getMaxNumHands(); i++){
		ofxOpenNIDepthThreshold depthThreshold = ofxOpenNIDepthThreshold(0, 0, false, true, true, true, true); 
		// ofxOpenNIDepthThreshold is overloaded, has defaults and can take a lot of different parameters, eg:
		// (ofxOpenNIROI OR) int _nearThreshold, int _farThreshold, bool _bUsePointCloud = false, bool _bUseMaskPixels = true, 
		// bool _bUseMaskTexture = true, bool _bUseDepthPixels = false, bool _bUseDepthTexture = false, 
		// int _pointCloudDrawSize = 2, int _pointCloudResolution = 2
		depthThreshold.setUsePointCloud(true);
		depthThreshold.setPointCloudDrawSize(2);

		depthStream.addDepthThreshold(depthThreshold);

	}

	depthStream.start();
}
#endif


ofVec3f testApp::Finger::getFilteredPosition(float a)
{
	ofVec3f res;
	
	for (int i = 0; i < position.size(); i++)
	{
		res += pow(1-a,i) * position[i];
	}
	res *= a;
	return res;
}
