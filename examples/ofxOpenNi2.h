#pragma once

#include "ofMain.h"
#include "ofPixels.h"

#include "openni.h"

class ofxVideoStream : public openni::VideoStream::Listener
{
public:

	void setup(openni::VideoMode& videoMode);
	void update();
	ofPixels& getPixels();

	virtual void onNewFrame( openni::VideoStream& ) = 0;
protected:
	virtual void allocatePixels(int w, int h) = 0;
	ofTexture texture;
	ofPixels pixels[2];
	
	openni::VideoStream videoStream;


};

class ofxColorStream : public ofxVideoStream
{
	virtual void onNewFrame( openni::VideoStream& );
	virtual void allocatePixels(int w, int h);
};

class ofxDepthStream : public ofxVideoStream
{
	virtual void onNewFrame( openni::VideoStream& );
	virtual void allocatePixels(int w, int h);
};


namespace ofxOpenNI
{
	int init();
};

class ofxOpenNIListener : public openni::OpenNI::Listener
{

};

class wtf
{


public:
	int setup(bool threaded = true);
	void update();


	

private: 
	openni::OpenNI openni;
	openni::Status rc;
	openni::Device device;
	
	ofxDepthStream depthStream;
	ofxColorStream colorStream;


	/*
	ofPixels& getDepthPixels();
	ofShortPixels& getDepthRawPixels();
	*/


	/*

	void toggleRegister();
	void setRegister(bool b);
	bool getRegister();

	void toggleMirror();
	void setMirror(bool b);
	bool getMirror();

	void toggleSync();
	void setSync(bool b);
	bool getSync();


	ofTexture& getDepthTextureReference();
	ofTexture& getColorTextureReference();

	ofPoint worldToProjective(const ofPoint & p);
	ofPoint worldToProjective(const XnVector3D & p);

	ofPoint getConvertedProjectiveToWorld(const ofPoint & p);
	ofPoint getConvertedProjectiveToWorld(const XnVector3D & p);
	void convertProjectiveToWorld(ofVec3f* p, int n);

	ofPoint cameraToWorld(const ofVec2f& c);
	void cameraToWorld(const vector<ofVec2f>& c, vector<ofVec3f>& w);

		
	int getDeviceID();

	string LOG_NAME;

	ofEvent<ofxOpenNIUserEvent> userEvent;
	ofEvent<ofxOpenNIGestureEvent> gestureEvent;
	ofEvent<ofxOpenNIHandEvent> handEvent;
	*/



};
