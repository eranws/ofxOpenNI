#include "ofxRecorder.h"
#include "OpenNI.h"


void ofxRecorder::setup( ofPtr<openni::VideoStream> stream )
{
	recorder = ofPtr<openni::Recorder>(new openni::Recorder);

}

void ofxRecorder::start()
{
	
	recorder->create(ofGetTimestampString().append(".oni").c_str()); 
	
	
	for (int i = 0; i < _streams.size(); i++)
	{
		recorder->attach(*_streams[i]);
	}
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

void ofxRecorder::addStream( ofPtr<openni::VideoStream> stream )
{
	//TODO: multistream
	_streams.push_back(stream);
}
