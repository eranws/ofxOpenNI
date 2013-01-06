#include "ofxRecorder.h"
#include "OpenNI.h"


void ofxRecorder::setup( ofPtr<openni::VideoStream> stream )
{
	recorder = ofPtr<openni::Recorder>(new openni::Recorder);

}

void ofxRecorder::start()
{
	
	recorder->create(ofGetTimestampString().append(".oni").c_str()); 
	recorder->attach(*_stream);
	// second parameter – allow lossy compression. Default is FALSE
	// Multiple streams can be attached – but only one from each type

	recorder->start();

}

void ofxRecorder::stop()
{
	recorder->stop();
	recorder->destroy();
}

void ofxRecorder::exit()
{
	stop();
}

void ofxRecorder::attach( ofPtr<openni::VideoStream> stream )
{
	//TODO: multistream
	_stream = stream;
}
