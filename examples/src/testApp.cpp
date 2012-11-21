#include "testApp.h"

#include <stdlib.h>

//--------------------------------------------------------------
void testApp::setup(){	

	ofSetVerticalSync(true);

	item.loadImage("graphics/falafel.png");
	itemSize = ofVec2f(item.getWidth(), item.getHeight());
	itemSizeFactor = 1.0f;

	bgImage.loadImage("graphics/bg.jpg"); //depends on resolution

	setGUI4();
}


//--------------------------------------------------------------
void testApp::update(){
	camString = stringstream();
	mg->addPoint(ofGetFrameRate());

	itemPos = ofVec2f(mouseX, mouseY);
}


//--------------------------------------------------------------
void testApp::draw()
{

	bgImage.draw(400,400);
	item.draw(itemPos, itemSize.x * itemSizeFactor, itemSize.y * itemSizeFactor);


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




//--------------------------------------------------------------
void testApp::exit(){

	delete gui4; 
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
	//gui4->addWidgetDown(new ofxUIImageSampler(img->getWidth(), img->getHeight(), img, "SAMPLER"));

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




