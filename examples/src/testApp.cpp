#include "testApp.h"

#include <stdlib.h>
#include "OpenNI.h"
using namespace openni;

//--------------------------------------------------------------
void testApp::setup(){	

	ofSetCircleResolution(100);
	//ofSetFrameRate(300);
	//	ofSetFullscreen(true);
	ofSetVerticalSync(true);

	setupOpenNi();

	img = new ofImage(); 
	img->getTextureReference().allocate(640,480, GL_DEPTH);
	img->setColor(320,240, ofColor::white);
	buffer = new float[256];     

	for(int i = 0; i < 256; i++) { buffer[i] = ofNoise(i/100.0); }
	setGUI4();

//	faceTracker.setup();

	start();


}


//--------------------------------------------------------------
void testApp::update(){
	camString = stringstream();
	mg->addPoint(ofGetFrameRate());

}


//--------------------------------------------------------------
void testApp::draw()
{
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

	cv::Mat m(depthPixels->getWidth(), depthPixels->getHeight(), CV_16UC1, depthPixels->getPixels());
	imshow("",m);



	static bool a = true;

	bool r = true;
	
	vector<ofxUIWidget*> w = gui4->getWidgets();
	for (int i = 0; i<w.size(); i++)
	{
		bool r = int(sinf(ofGetSystemTime() / 500)) % 2 == 0;
		bool highestFrequency = ofGetSystemTimeMicros() % 2;
		bool randBool = rand() % 2;

		w[i]->setVisible(r);
	}
	cv::Mat m2(depthPixels->getWidth(), depthPixels->getHeight(), CV_8UC1, depthPixels->getPixels(), 16);
	imshow("", m2);



	ofSetHexColor(0xffffff);
	depthTexture.draw(0,0, depthTexture.getWidth(), depthTexture.getHeight());





	ofCircle(mouseX, mouseY,20);

	ofSetHexColor(0x333333);
	ofDrawBitmapString("fps:" + ofToString(ofGetFrameRate()), 10,10);

}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 

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

	//notify face?
}



//--------------------------------------------------------------
void testApp::exit(){

	delete gui4; 

	faceTracker.stopThread();
	faceTracker.waitForThread();

	depthStream.removeListener(this);
	depthStream.stop();
	depthStream.destroy();
	device.close();

	OpenNI::shutdown();

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
	gui4 = new ofxUIScrollableCanvas(length*3+xInit*3+6, 0, length+xInit, ofGetHeight());     
	gui4->addWidgetDown(new ofxUILabel("PANEL 4: SCROLLABLE", OFX_UI_FONT_LARGE)); 	

	gui4->addSpacer(length-xInit, 2);

	gui4->addWidgetDown(new ofxUILabel("BILABEL SLIDER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUIBiLabelSlider(length-xInit, 0, 100, 50, "BILABEL", "HOT", "COLD", OFX_UI_FONT_MEDIUM));

	gui4->addWidgetDown(new ofxUILabel("MINIMAL SLIDER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUIMinimalSlider(length-xInit, dim, 0, 100, 50.0, "MINIMAL",OFX_UI_FONT_MEDIUM));

	gui4->addSpacer(length-xInit, 2);

	gui4->addWidgetDown(new ofxUILabel("CIRCLE SLIDER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUICircleSlider((length-xInit)*.5, 0, 100, 50.0, "NORTH SOUTH", OFX_UI_FONT_MEDIUM));    

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

	gui4->addSpacer(length-xInit, 2);
	gui4->addWidgetDown(new ofxUILabel("IMAGE SAMPLER", OFX_UI_FONT_MEDIUM)); 				
	gui4->addWidgetDown(new ofxUIImageSampler(img->getWidth(), img->getHeight(), img, "SAMPLER"));

	gui4->addWidgetDown(new ofxUIMultiImageButton(dim*2, dim*2, false, "GUI/toggle.png", "IMAGE BUTTON"));
	gui4->addWidgetDown(new ofxUIMultiImageToggle(dim*2, dim*2, false, "GUI/toggle.png", "IMAGE BUTTON"));


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
	else if(name == "DRAW GRID")
	{
		ofxUIButton *button = (ofxUIButton *) e.widget; 
		bdrawGrid = button->getValue(); 
	}
	else if(name == "D_GRID")
	{
		ofxUIToggle *toggle = (ofxUIToggle *) e.widget; 
		bdrawGrid = toggle->getValue(); 
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


