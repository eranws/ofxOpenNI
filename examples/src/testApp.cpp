#include "testApp.h"

#include <stdlib.h>
#include "OpenNI.h"
using namespace openni;


void updateUserState(const nite::UserData& user, unsigned long long ts);

//--------------------------------------------------------------
void testApp::setup(){	

	ofSetCircleResolution(100);
	//ofSetFrameRate(300);
	//	ofSetFullscreen(true);
	ofSetVerticalSync(true);

	setupOpenNi();
	setupNite();

	img = new ofImage(); 
	img->getTextureReference().allocate(640,480, GL_DEPTH);
	img->setColor(320,240, ofColor::white);
	buffer = new float[256];     

	for(int i = 0; i < 256; i++) { buffer[i] = ofNoise(i/100.0); }
	setGUI4();

	start();
}


//--------------------------------------------------------------
void testApp::update(){
	camString = stringstream();
	mg->addPoint(ofGetFrameRate());

	//	onNewFrame(depthStream);
}


//--------------------------------------------------------------
void testApp::draw(){

	ofShortPixels* depthPixels = depthPixelsDoubleBuffer[0];

	unsigned short* p = depthPixels->getPixels();
	for (int i=0; i < depthPixels->size(); i++)
	{
		unsigned short& k = p[i];

		{
			colorPixels[3*i + 0] = (k >> 5) & 0xff;
			colorPixels[3*i + 1] = (k >> 3) & 0xff;
			colorPixels[3*i + 2] = k & 0xff;// & 0xff;

		}
	}

	depthTexture.loadData(colorPixels);

	if (drawDebug && drawOpenNiDebug)
	{
		ofSetHexColor(0xffffff);
		depthTexture.draw(0,0, depthTexture.getWidth(), depthTexture.getHeight());
	}


<<<<<<< HEAD
	const nite::Array<nite::HandData>& hands = handTrackerFrame[0].getHands();
	for (int i = 0; i < hands.getSize(); ++i)
	{
		const nite::HandData& hand = hands[i];
		if (hand.getState() != NITE_HAND_STATE_LOST)
		{

			if (drawDebug && drawOpenNiDebug)
			{
				ofVec2f handScreenPos;
				handTracker.convertHandCoordinatesToDepth(hand.getPosition().x,hand.getPosition().y,hand.getPosition().z,&handScreenPos.x, &handScreenPos.y);
				ofCircle(ofPoint(handScreenPos), 10);
				ofDrawBitmapStringHighlight(ofToString(hand.getId()), handScreenPos);
			}

			//		printf("%d. (%5.2f, %5.2f, %5.2f)\n", hand.getId(), hand.getPosition().x, hand.getPosition().y, hand.getPosition().z);
		}
	}

	bgImage.draw(400,400);
	item.draw(itemPos, itemSize.x * itemSizeFactor, itemSize.y * itemSizeFactor);

	ofCircle(ofPoint(headScreenPos), 10);

	ofSetHexColor(0x333333);
	ofDrawBitmapString("fps:" + ofToString(ofGetFrameRate()), 10,10);

}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 

	switch (key) 
	{	
		case 'g': gui4->toggleVisible(); break;
	}

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
	if (x < 30 && y < 30) gui4->setVisible(true);
	if (gui4->isVisible() && !gui4->isHit(x,y)) gui4->setVisible(false);
	
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


//TODO: exception?
int testApp::setupOpenNi()
{
	Status rc = OpenNI::initialize();
	if (rc != ONI_STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}
	OpenNI::addListener(this);

	rc = device.open(ONI_ANY_DEVICE);
	if (rc != ONI_STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());

		printf("opening file");
		rc = device.open("C:\\f\\q.oni");
		if (rc != ONI_STATUS_OK)
		{
			return 2;
		}
	}


	if (device.getSensorInfo(openni::SENSOR_DEPTH) != NULL)
	{
		rc = depthStream.create(device, SENSOR_DEPTH);
		if (rc != ONI_STATUS_OK)
		{
			printf("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
		}
	}


	//	stream.readFrame(&frame);


	int w = depthStream.getVideoMode().getResolutionX();
	int h = depthStream.getVideoMode().getResolutionY();

	depthTexture.allocate(w, h, GL_RGB);


	for (int i=0 ; i<2; i++)
	{
		depthPixelsDoubleBuffer[i] = new ofShortPixels();
		depthPixelsDoubleBuffer[i]->allocate(w, h, OF_IMAGE_GRAYSCALE);
	}
	colorPixels.allocate(w, h, OF_IMAGE_COLOR);

	//	onNewFrame(depthStream);

	return 0;

}

void testApp::onNewFrame( VideoStream& stream )
{
	stream.readFrame(&frame);
	const unsigned short* data = (const unsigned short*)frame.getData();

	bool debugPrintMiddlePixel = false;
	if (debugPrintMiddlePixel) //TODO: move to Utils
	{
		int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;
		DepthPixel* pDepth = (DepthPixel*)frame.getData();
		printf("[%08llu] %8d\n", frame.getTimestamp(), pDepth[middleIndex]);
	}

	depthPixelsDoubleBuffer[1]->setFromPixels(data, frame.getWidth(), frame.getHeight(), OF_IMAGE_GRAYSCALE);

	depthPixelsDoubleBuffer[0] = depthPixelsDoubleBuffer[1];
	//InterlockedExchangePointer(depthPixelsDoubleBuffer[0],depthPixelsDoubleBuffer[1]);

}

	
	nite::Status niteRc = userTracker.readFrame(&userTrackerFrame);

	if (niteRc != NITE_STATUS_OK)
	{
		printf("Get next frame failed\n");
	}


	const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();
	for (int i = 0; i < users.getSize(); ++i)
	{
		const nite::UserData& user = users[i];
		updateUserState(user,userTrackerFrame.getTimestamp());
		if (user.getState() == nite::USER_STATE_NEW)
		{
			userTracker.startSkeletonTracking(user.getId());
		}
		else if (user.getSkeleton().getState() == nite::SKELETON_TRACKED)
		{
			const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);
			if (head.getPositionConfidence() > .5)
			{
				userTracker.convertJointCoordinatesToDepth(head.getPosition().x,head.getPosition().y,head.getPosition().z,&headScreenPos.x, &headScreenPos.y);
				printf("%d. (%5.2f, %5.2f, %5.2f)\n", user.getId(), head.getPosition().x, head.getPosition().y, head.getPosition().z);
			}

		}
	}
}



//--------------------------------------------------------------
void testApp::exit(){

	delete gui4; 

	depthStream.removeListener(this);
	depthStream.stop();
	depthStream.destroy();
	device.close();

	abort(); //avoid crash in tite tracker. TODO remove
	nite::NiTE::shutdown();
	OpenNI::shutdown();

}

int testApp::setupNite()
{
	nite::Status niteRc;

	niteRc = nite::NiTE::initialize();
	if (niteRc != NITE_STATUS_OK)
	{
		printf("NiTE initialization failed\n");
		return 1;
	}

	niteRc = userTracker.create();
	if (niteRc != NITE_STATUS_OK)
	{
		printf("Couldn't create user tracker\n");
		return 3;
	}


	return 0;
}

int testApp::start()
{
	Status rc = depthStream.start();
	if (rc != ONI_STATUS_OK)
	{
		printf("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
	}

	// Register to new frame
	rc = depthStream.addListener(this);
	if (rc != ONI_STATUS_OK)
	{
		printf("Couldn't register listener for the depth stream\n%s\n", OpenNI::getExtendedError());
	}


	return 0;

}



void testApp::setGUI4()
{	
	float dim = 16; 
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
	float length = 255-xInit; 
	gui4 = new ofxUIScrollableCanvas(0, 0, length+xInit, ofGetHeight());     
	gui4->addWidgetDown(new ofxUILabel("PANEL 4: SCROLLABLE", OFX_UI_FONT_LARGE)); 	

	gui4->addSpacer(length-xInit, 2);

	/*
	gui4->addWidgetDown(new ofxUILabel("BILABEL SLIDER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUIBiLabelSlider(length-xInit, 0, 100, 50, "BILABEL", "HOT", "COLD", OFX_UI_FONT_MEDIUM));

	gui4->addWidgetDown(new ofxUILabel("MINIMAL SLIDER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUIMinimalSlider(length-xInit, dim, 0, 100, 50.0, "MINIMAL",OFX_UI_FONT_MEDIUM));

	gui4->addSpacer(length-xInit, 2);

	gui4->addWidgetDown(new ofxUILabel("CIRCLE SLIDER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUICircleSlider((length-xInit)*.5, 0, 100, 50.0, "NORTH SOUTH", OFX_UI_FONT_MEDIUM));    
	*/
	gui4->addSpacer(length-xInit, 2);
	gui4->addWidgetDown(new ofxUILabel("FPS SLIDER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addFPSSlider("FPS SLIDER", length-xInit, dim);

	vector<float> buffer; 
	for(int i = 0; i < 256; i++)
	{
		buffer.push_back(0.0);
	}

	gui4->addWidgetDown(new ofxUILabel("MOVING GRAPH", OFX_UI_FONT_MEDIUM)); 				    
	mg = (ofxUIMovingGraph *) gui4->addWidgetDown(new ofxUIMovingGraph(length-xInit, 120, buffer, 256, 0, 400, "MOVING GRAPH"));

	/*
	gui4->addSpacer(length-xInit, 2);
	gui4->addWidgetDown(new ofxUILabel("IMAGE SAMPLER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUIImageSampler(img->getWidth(), img->getHeight(), img, "SAMPLER"));

	gui4->addWidgetDown(new ofxUIMultiImageButton(dim*2, dim*2, false, "GUI/toggle.png", "IMAGE BUTTON"));
	gui4->addWidgetDown(new ofxUIMultiImageToggle(dim*2, dim*2, false, "GUI/toggle.png", "IMAGE BUTTON"));
	*/

	//gui4->addWidgetDown(new ofxUILabel("BUTTONS", OFX_UI_FONT_MEDIUM)); 
	gui4->addToggle("DRAW DEBUG", false, dim, dim);
	//gui4->addWidgetDown(new ofxUILabel("TOGGLES", OFX_UI_FONT_MEDIUM)); 
	gui4->addToggle( "DRAW DEBUG OpenNI", false, dim, dim);


	ofAddListener(gui4->newGUIEvent,this,&testApp::guiEvent);
}


void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName(); 
	int kind = e.widget->getKind(); 
	cout << "got event from: " << name << endl; 	

	if(name == "RED")
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget; 
		cout << "RED " << slider->getScaledValue() << endl; 
		red = slider->getScaledValue(); 
	}
	else if(name == "GREEN")
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget; 
		cout << "GREEN " << slider->getScaledValue() << endl; 
		green = slider->getScaledValue(); 
	}

	else if(name == "BLUE")
	{
		ofxUISlider *slider = (ofxUISlider *) e.widget; 
		cout << "BLUE " << slider->getScaledValue() << endl; 
		blue = slider->getScaledValue(); 		
	}
	else if(name == "DRAW DEBUG")
	{
		ofxUIButton *button = (ofxUIButton *) e.widget; 
		drawDebug = button->getValue(); 
	}
	else if(name == "DRAW DEBUG OpenNI")
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget; 
		drawOpenNiDebug = toggle->getValue(); 
	}
	else if(name == "TEXT INPUT")
	{
		ofxUITextInput *textinput = (ofxUITextInput *) e.widget; 
		if(textinput->getTriggerType() == OFX_UI_TEXTINPUT_ON_ENTER)
		{
			cout << "ON ENTER: "; 
			//            ofUnregisterKeyEvents((testApp*)this); 
		}
		else if(textinput->getTriggerType() == OFX_UI_TEXTINPUT_ON_FOCUS)
		{
			cout << "ON FOCUS: "; 
		}
		else if(textinput->getTriggerType() == OFX_UI_TEXTINPUT_ON_UNFOCUS)
		{
			cout << "ON BLUR: "; 
			//            ofRegisterKeyEvents(this);             
		}        
		string output = textinput->getTextString(); 
		cout << output << endl; 
	}
}



#define MAX_USERS 10
nite::UserState g_userStates[MAX_USERS] = {(nite::UserState)-1};
nite::SkeletonState g_skeletonStates[MAX_USERS] = {nite::SKELETON_NONE};

#define USER_MESSAGE(msg) \
{printf("[%08llu] User #%d:\t%s\n",ts, user.getId(),msg); break;}

void updateUserState(const nite::UserData& user, unsigned long long ts)
{
	if(g_userStates[user.getId()] != user.getState())
	{
		switch(g_userStates[user.getId()] = user.getState())
		{
		case nite::USER_STATE_NEW:
			USER_MESSAGE("New")
		case nite::USER_STATE_VISIBLE:
			USER_MESSAGE("Visible")
		case nite::USER_STATE_OUT_OF_SCENE:
			USER_MESSAGE("Out of scene...")
		case nite::USER_STATE_LOST:
			USER_MESSAGE("Lost.")
		}
	}
	if(g_skeletonStates[user.getId()] != user.getSkeleton().getState())
	{
		switch(g_skeletonStates[user.getId()] = user.getSkeleton().getState())
		{
		case nite::SKELETON_NONE:
			USER_MESSAGE("Stopped tracking.")
		case nite::SKELETON_CALIBRATING:
			USER_MESSAGE("Calibrating...")
		case nite::SKELETON_TRACKED:
			USER_MESSAGE("Tracking!")
		case nite::SKELETON_CALIBRATION_ERROR_NOT_IN_POSE:
		case nite::SKELETON_CALIBRATION_ERROR_HANDS:
		case nite::SKELETON_CALIBRATION_ERROR_LEGS:
		case nite::SKELETON_CALIBRATION_ERROR_HEAD:
		case nite::SKELETON_CALIBRATION_ERROR_TORSO:
			USER_MESSAGE("Calibration Failed... :-|")
		}
	}
}
