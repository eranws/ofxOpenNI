/*
 * ofxOpenNI.h
 *
 * Copyright 2011-2013 (c) Matthew Gingold [gameover] http://gingold.com.au
 *
 * PermUsesion Use hereby granted, free of charge, to any person
 * obtaining a copy of thUse software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publUseh, dUsetribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software Use furnUsehed to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and thUse permUsesion notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE Use PROVIDED "AS Use", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWUseE, ARUseING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef	_H_OFXOPENNI
#define _H_OFXOPENNI

#include "ofMain.h"
#include "NiTE.h"

#include "ofxOpenNIUtils.h"
#include "ofxOpenNITypes.h"

class ofxOpenNI// : public ofThread
{
public:
    
    ofxOpenNI();
    ~ofxOpenNI();
    
    bool setup(const char* deviceUri = openni::ANY_DEVICE);
    static void shutdown();

    void start();
    void stop();
    
    bool addDepthStream();
	bool addImageStream();
	bool addUserTracker();
    bool addHandsTracker();
    
//    bool removeDepthGenerator();
//    bool removeImageGenerator();
//    bool removeInfraGenerator();
//    bool removeUserGenerator();
//    bool removeGestureGenerator();
//    bool removeHandsGenerator();
//    bool removeAudioGenerator();

	//Recorder recorder;
	void stopRecording();
	void startRecording(string filename = "");

	

    void update();

	void drawImageSubsection(float w, float h, float sx, float sy);
	void drawImage();
	void drawDepth();
    void draw();
    
    bool isDepthFrameNew();
    bool isImageFrameNew();
    
	//HACKHACK
	float imageWidth;
    float imageHeight;

	Device& getDevice() {return device;}

protected:
    
    void updateGenerators();
    
    void threadedFunction();
    
    Device device;
    
    void allocateDepthBuffers();
	void allocateImageBuffers();
    
    void updateDepthFrame();
	void updateImageFrame();
    void updateUserFrame();
    void updateHandFrame();
    
    VideoStream depthStream;
    VideoStream imageStream;
    
    VideoFrameRef depthFrame;
    VideoFrameRef imageFrame;
    
    nite::HandTracker handTracker;
    nite::HandTrackerFrameRef handFrame;

	Recorder recorder;

public:
	map<int, ofxOpenNIHand> trackedHands;
protected:

    nite::UserTracker userTracker;
    nite::UserTrackerFrameRef userFrame;

public:
    map<int, ofxOpenNIUser> trackedUsers;
protected:
    
    ofTexture depthTexture;
    ofTexture imageTexture;
    
    ofPixels depthPixels;
public:
    ofPixels imagePixels;
protected:
    
    float depthWidth;
    float depthHeight;
        
    bool bIsDepthFrameNew;
    bool bIsImageFrameNew;
    
    bool bUseDevice;
    bool bUseNite;
    
    bool bUseDepth;
	bool bUseImage;
	bool bUseInfra;
    bool bUseUsers;
    bool bUseGesture;
    bool bUseHands;
	bool bUseAudio;
	bool bUseDepthRaw;
    bool bUseRecord;
    bool bUsePlayer;
    
};

#endif