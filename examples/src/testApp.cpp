#include "testApp.h"

#include <stdlib.h>
#include "OpenNI.h"
using namespace openni;
using namespace nite;



bool operator<(XHead& a, XHead& b)
{
	return a.pos.z < b.pos.z;
}

//--------------------------------------------------------------
void testApp::setup(){	

	_closing = false;

	ofSetCircleResolution(100);
	//ofSetFrameRate(300);
	ofSetVerticalSync(true);

//	glEnable(GL_DEPTH_TEST);
	for (int i = 0; i < 2; i++)
	{
		headMaps[i] = new HeadMap;
	}
	setupOpenNi();
	setupNite();

	item.loadImage("graphics/falafel.gif");
	item.setAnchorPercent(0.5, 0.6); 
	itemSize = ofVec2f(item.getWidth(), item.getHeight());


	for (int i=0; i<handImageCount; i++)
	{
		stringstream ss;
		ss << "graphics/hand/hand-" << i + 1 << ".gif";
		hand[i].loadImage(ss.str());
		hand[i].setAnchorPercent(0.7, 0.5); 
		//itemSize = ofVec2f(item.getWidth(), item.getHeight());

	}
	handSize = ofVec2f(hand[0].getWidth(), hand[0].getHeight());
	handSizeFactor = 2.0f;


	text.loadImage("graphics/text/text-1.gif");
	text.setAnchorPercent(0.5, 0.5); 
	textSize = ofVec2f(text.getWidth(), text.getHeight());
	textSizeFactor = 0.4f;


	for (int i=0; i<instImageCount; i++)
	{
		stringstream ss;
		ss << "graphics/walkinginstruction/walking-instruction" << i << ".gif";
		inst[i].loadImage(ss.str());
		inst[i].setAnchorPercent(1, 1); 
	}

	instSize = ofVec2f(inst[0].getWidth(), inst[0].getHeight());
	instSizeFactor = 1.0f;


	//Crown
	crown.loadImage("graphics/crown.gif");
	crown.setAnchorPercent(0.5, 1.0);
	crownSize = ofVec2f(crown.getWidth(), crown.getHeight());
	crownSizeFactor = 0.7f;


	for (int i=0; i<bgImageCount; i++)
	{
		stringstream ss;
		ss << "graphics/2ndscreen/falafel-kingdom-" << i+1 << ".gif"; //depends on resolution
		bgImage[i].loadImage(ss.str());
		bgImage[i].setAnchorPercent(0, 0); 
	}


	drawColorBackground = true;
	bgProgress = 0.0f;
	animateBg = false;

	windowResized(ofGetWindowWidth(), ofGetWindowHeight());
	setGUI4();

	start();
}


//--------------------------------------------------------------
void testApp::update(){
	debugString = stringstream();
	mg->addPoint(ofGetFrameRate());


	if (depthStream.isValid())
	{
		ofShortPixels* depthPixels = depthPixelsDoubleBuffer[0];

		ofPixels depthColorPixels;
		depthColorPixels.allocate(depthPixels->getWidth(), depthPixels->getHeight(), OF_IMAGE_COLOR);

		unsigned short* p = depthPixels->getPixels();
		for (int i=0; i < depthPixels->size(); i++)
		{
			unsigned short& k = p[i];

			{
				depthColorPixels[3*i + 0] = (k >> 5) & 0xff;
				depthColorPixels[3*i + 1] = (k >> 3) & 0xff;
				depthColorPixels[3*i + 2] = k & 0xff;// & 0xff;
			}
		}
		depthTexture.loadData(depthColorPixels);



		ofShortPixels* userPixels = userPixelsDoubleBuffer[0];
		ofPixels userColorPixels;
		userColorPixels.allocate(userPixels->getWidth(), userPixels->getHeight(), OF_IMAGE_COLOR);

		unsigned short* u = userPixels->getPixels();
		for (int i=0; i < userPixels->size(); i++)
		{
			unsigned short& k = u[i];
			{
				userColorPixels[3*i + 0] = k * 50;
				userColorPixels[3*i + 1] = k * 50;
				userColorPixels[3*i + 2] = k * 50;
			}
		}
		usersTexture.loadData(userColorPixels);
	}



}


//--------------------------------------------------------------

void testApp::draw()
{
	//	bgImage.draw(400,400);
	if (0 && drawDebug && drawOpenNiDebug)
	{
		ofSetHexColor(0xffffff);
		depthTexture.draw(100,100);
		ofSetHexColor(0x333333);
		ofDrawBitmapStringHighlight("fps:" + ofToString(ofGetFrameRate()), 10,10);
	}


	ofSetHexColor(0xffffff);

	if (drawColorBackground)
	{
		colorTexture.draw(0, 0, 0, ofGetWindowWidth(), ofGetWindowHeight());
	}

	int bgIndex = (ofGetElapsedTimeMillis() % 1000) / 500;
	if (animateBg) //state 2
	{
		if (bgProgress < 1.0f)
		{
			bgProgress += 0.02f;
		}
		else
		{
			drawColorBackground = false;
			bgImage[bgIndex].draw(0, 0, bgSize.x, bgSize.y);
		}

		if (bgProgress > 1.0f) bgProgress = 1.0f;
		bgImage[bgIndex].setAnchorPercent(-1.0 + bgProgress, 0);

	}
	else
	{
		bgProgress = 0.0;
		drawColorBackground = true;
	}

	if (drawColorBackground && colorStream.isValid())
	{
		ofPixels* colorPixels = colorPixelsDoubleBuffer[0];
		colorTexture.loadData(*colorPixels);

		HeadMap* headMap0 = headMaps[0];
		HeadMap headMap = *headMap0;

		sort(headMap.begin(), headMap.end());
		
		for (HeadMap::reverse_iterator it = headMap.rbegin(); it != headMap.rend(); it++)
		{
			ofShortPixels* frontUserPixels = userPixelsDoubleBuffer[0];
			ofPixels frontUserColorPixels;
			frontUserColorPixels.allocate(frontUserPixels->getWidth(), frontUserPixels->getHeight(), OF_IMAGE_COLOR_ALPHA);

			unsigned short* u = frontUserPixels->getPixels();
			unsigned char* c = colorPixels->getPixels();

			for (int i=0; i < frontUserPixels->size(); i++)
			{
				unsigned short& k = u[i];

				if (k == it->id)
				{
					frontUserColorPixels[4*i + 0] = c[3*i + 0];
					frontUserColorPixels[4*i + 1] = c[3*i + 1];
					frontUserColorPixels[4*i + 2] = c[3*i + 2];
					frontUserColorPixels[4*i + 3] = 0xff;

				}
				else
				{
					frontUserColorPixels[4*i + 0] = 0;
					frontUserColorPixels[4*i + 1] = 0;
					frontUserColorPixels[4*i + 2] = 0;
					frontUserColorPixels[4*i + 3] = 0;

				}
			}
			frontUserColorTexture.loadData(frontUserColorPixels);


			if (animateBg && it->id == headMap[0].id)
			{
				bgImage[bgIndex].draw(0, 0, bgSize.x, bgSize.y);
			}

			ofPoint poss(0, 0, -it->pos.z);
			frontUserColorTexture.draw(poss, ofGetWindowWidth(), ofGetWindowHeight());



			ofPoint pos;

			pos.x = it->pos.x * ofGetWindowWidth() / colorStream.getVideoMode().getResolutionX();
			pos.y = it->pos.y * ofGetWindowHeight() / colorStream.getVideoMode().getResolutionY();
			pos.z = it->pos.z;

			//ofCircle(pos, 20); //debug

			if (it->id == headMap[0].id)
			{

				float f = 1000.0 / it->pos.z;
				crown.draw(pos, crownSize.x * crownSizeFactor * f, crownSize.y * crownSizeFactor * f);

				if (pos.x > 0.9f * ofGetWindowWidth())
				{
					if (!animateBg)
					{
						toggleState(); 
					}

				}
			}
			else
			{
				float itemSizeFactor = falafelSizeFactor * 1000 / it->pos.z; //according to head distance
				item.draw(pos, itemSize.x * itemSizeFactor, itemSize.y * itemSizeFactor);
			}
		}

		int handIndex = (ofGetElapsedTimeMillis() % 1000) / 250;

		//float offset = abs(sinf((ofGetElapsedTimeMillis() % 1000) * 6.28f / 1000.0f ));
		//offset *= hand[0].getWidth() / 100;
		float offset = 0;
		hand[handIndex].draw(handPos.x - offset, handPos.y, handSize.x * handSizeFactor, handSize.y * handSizeFactor);

		if (ofGetFrameNum() % 100 == 0)
		{
			textAnimation = ofVec2f();
			textAnimationTarget = ofVec2f(ofGetWindowWidth() * 2, 0);
		}
		else
		{
			if (ofGetFrameNum() % 10 == 0)
			{
				textAnimationTarget = ofVec2f(ofRandomf(), ofRandomf()) * 10;
			}
		}
		
		textAnimation.interpolate(textAnimationTarget, 0.2f);

		text.draw(textPos + textAnimation, textSize.x * textSizeFactor, textSize.y * textSizeFactor);


		// walking instructions
		int instIndex = (ofGetElapsedTimeMillis() % 1998) / 111;
		if (instIndex < instImageCount)
		{
			inst[instIndex].draw(instPos.x, instPos.y, instSize.x * instSizeFactor, instSize.y * instSizeFactor);
		}


	}
}
//--------------------------------------------------------------
void testApp::keyPressed(int key){ 

	switch (key) 
	{	
	case 'f': ofToggleFullscreen(); break;
	case 'g': gui4->toggleVisible(); break;
	case ' ': toggleState(); break;

	}

}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y )
{
	if (x < 30 && y < 30) gui4->setVisible(true);
	if (gui4->isVisible() && !gui4->isHit(x,y)) gui4->setVisible(false);
}


//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	textPos = ofVec2f(ofGetWindowWidth() * 200.0f/290.0f, ofGetWindowHeight() * 100.0f/210.0f);
	handPos = ofVec2f(ofGetWindowWidth(), ofGetWindowHeight() * 0.7 );
	instPos = ofVec2f(ofGetWindowWidth(), ofGetWindowHeight());

	bgSize = ofVec2f(ofGetWindowWidth(), ofGetWindowHeight());
}


int testApp::setupOpenNi()
{
	openni::Status rc = OpenNI::initialize();
	if (rc != ONI_STATUS_OK)
	{
		printf("Initialize failed\n%s\n", OpenNI::getExtendedError());
		return 1;
	}
	OpenNI::addListener(this);

	rc = device.open(0);
	if (rc != ONI_STATUS_OK)
	{
		printf("Couldn't open device\n%s\n", OpenNI::getExtendedError());

		const string fn = "C:\\f\\q.oni";
		cout << ("opening file ") << fn;
		rc = device.open(fn.c_str());
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


	if (device.getSensorInfo(openni::SENSOR_COLOR) != NULL)
	{
		rc = colorStream.create(device, SENSOR_COLOR);
		if (rc != ONI_STATUS_OK)
		{
			printf("Couldn't create color stream\n%s\n", OpenNI::getExtendedError());
		}
	}

	rc = device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
	if (rc != ONI_STATUS_OK)
	{
		printf("setImageRegistrationMode Error: \n%s\n", OpenNI::getExtendedError());
	}



	int dw = depthStream.getVideoMode().getResolutionX();
	int dh = depthStream.getVideoMode().getResolutionY();
	depthTexture.allocate(dw, dh, GL_RGB);
	usersTexture.allocate(dw, dh, GL_RGB);
	frontUserColorTexture.allocate(dw, dh, GL_RGBA);

	int cw = colorStream.getVideoMode().getResolutionX();
	int ch = colorStream.getVideoMode().getResolutionY();
	colorTexture.allocate(dw, dh, GL_RGB);


	for (int i=0 ; i<2; i++)
	{
		depthPixelsDoubleBuffer[i] = new ofShortPixels();
		depthPixelsDoubleBuffer[i]->allocate(dw, dh, OF_IMAGE_GRAYSCALE);

		colorPixelsDoubleBuffer[i] = new ofPixels();
		colorPixelsDoubleBuffer[i]->allocate(cw, ch, OF_IMAGE_COLOR);

		userPixelsDoubleBuffer[i] = new ofShortPixels();
		userPixelsDoubleBuffer[i]->allocate(dw, dh, OF_IMAGE_GRAYSCALE);

	}

	return 0;
}

void testApp::onNewFrame( VideoStream& stream )
{
	if (_closing || !stream.isValid()) return;

	VideoFrameRef frame;
	stream.readFrame(&frame);

	switch (frame.getVideoMode().getPixelFormat())
	{
	case PIXEL_FORMAT_RGB888:
		{

			colorStream.readFrame(&colorFrame);
			unsigned char* colorData = (unsigned char*)colorFrame.getData();
			colorPixelsDoubleBuffer[1]->setFromPixels(colorData , colorFrame.getWidth(), colorFrame.getHeight(), OF_IMAGE_COLOR);
			swap(colorPixelsDoubleBuffer[0],colorPixelsDoubleBuffer[1]);
			break;
		}


	case PIXEL_FORMAT_DEPTH_1_MM:
	case PIXEL_FORMAT_DEPTH_100_UM:
		{

			stream.readFrame(&depthFrame);
			const unsigned short* data = (const unsigned short*)depthFrame.getData();
			depthPixelsDoubleBuffer[1]->setFromPixels(data, depthFrame.getWidth(), depthFrame.getHeight(), OF_IMAGE_GRAYSCALE);
			swap(depthPixelsDoubleBuffer[0],depthPixelsDoubleBuffer[1]);

			nite::Status niteRc = userTracker->readFrame(&userTrackerFrame);
			if (niteRc != NITE_STATUS_OK)
			{
				printf("Get next frame failed\n");
			}

			const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();

			const nite::UserMap userMap = userTrackerFrame.getUserMap();

			const unsigned short* userData = (const unsigned short*)userMap.getPixels();
			userPixelsDoubleBuffer[1]->setFromPixels(userData, userMap.getWidth(), userMap.getHeight(), OF_IMAGE_GRAYSCALE);
			swap(userPixelsDoubleBuffer[0], userPixelsDoubleBuffer[1]);

			
			HeadMap* headMap = headMaps[1]; 
			headMap->clear();

			for (int i = 0; i < users.getSize(); ++i)
			{
				const nite::UserData& user = users[i];
				if (user.isNew())
				{
					userTracker->startSkeletonTracking(user.getId());
					//printf("%d.", user.getId());
				}
				else if (user.getSkeleton().getState() == nite::SKELETON_TRACKED)
				{
					const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);
					if (head.getPositionConfidence() > .5)
					{
						ofVec2f headScreenPos;
						userTracker->convertJointCoordinatesToDepth(head.getPosition().x,head.getPosition().y,head.getPosition().z,&headScreenPos.x, &headScreenPos.y);
						//printf("%d. (%5.2f, %5.2f, %5.2f)\n", user.getId(), head.getPosition().x, head.getPosition().y, head.getPosition().z);
						XHead xhead;
						xhead.id = user.getId();
						xhead.pos = ofPoint(headScreenPos.x, headScreenPos.y, head.getPosition().z);
						headMap->push_back(xhead);
					}



				}

			}
			swap(headMaps[0], headMaps[1]);


		}
	}
}



//--------------------------------------------------------------
void testApp::exit(){

	delete gui4; 

	//if(userTracker) delete(userTracker);
	_closing = true;
	Sleep(500);

	/*
	depthStream.removeListener(this);
	colorStream.removeListener(this);

	depthStream.stop();
	colorStream.stop();

	depthStream.destroy();
	colorStream.destroy();
	*/

	device.close();


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

	userTracker = new nite::UserTracker;
	niteRc = userTracker->create();
	if (niteRc != NITE_STATUS_OK)
	{
		printf("Couldn't create user tracker\n");
		return 3;
	}

	for (int i=0 ; i<2; i++)
	{
		//		userTrackerFrame[i] = new nite::UserTrackerFrameRef; 
	}

	return 0;
}

int testApp::start()
{
	openni::Status rc = depthStream.start();
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


	rc = colorStream.start();
	if (rc != ONI_STATUS_OK)
	{
		printf("Couldn't start the color stream\n%s\n", OpenNI::getExtendedError());
	}

	// Register to new frame
	rc = colorStream.addListener(this);
	if (rc != ONI_STATUS_OK)
	{
		printf("Couldn't register listener for the color stream\n%s\n", OpenNI::getExtendedError());
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
	*/

	gui4->addWidgetDown(new ofxUILabel("Falafel Size", OFX_UI_FONT_MEDIUM));

	falafelSizeFactor = 0.5;
	gui4->addWidgetDown(new ofxUICircleSlider((length-xInit)*.5, 0.2, 2.0, &falafelSizeFactor, "FALAFEL SIZE", OFX_UI_FONT_MEDIUM));    


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
	else if(name == "FALAFEL SIZE")
	{
		ofxUICircleSlider *cs= (ofxUICircleSlider *) e.widget; 
		printf("%.2f\n", cs->getScaledValue()); 
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

void testApp::toggleState()
{
	animateBg = !animateBg;
}



