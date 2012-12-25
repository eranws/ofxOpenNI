#include "ofxDepthStream.h"

#include "OpenNI.h"


void ofxDepthStream::setup(ofPtr<openni::Device> device)
{
	this->device = device;

	openni::Status rc;

	if (device->getSensorInfo(openni::SENSOR_DEPTH) != NULL)
	{
		stream = ofPtr<openni::VideoStream>(new openni::VideoStream);
		rc = stream->create(*device, openni::SENSOR_DEPTH);
		if (rc != openni::STATUS_OK)
		{
			throw ("Couldn't create depth stream\n%s\n", openni::OpenNI::getExtendedError());
		}
	}

	rc = stream->start();
	if (rc != openni::STATUS_OK)
	{
		throw ("Couldn't start the depth stream\n%s\n", openni::OpenNI::getExtendedError());

	}

	startThread(false, true);
}

void ofxDepthStream::exit()
{
	stopThread();
	waitForThread();
}

void ofxDepthStream::threadedFunction()
{
	openni::Status rc;

	openni::VideoFrameRef frame;
	while (isThreadRunning())
	{
		rc = stream->readFrame(&frame);
		if (rc != openni::STATUS_OK)
		{
			printf("Wait failed\n");
			continue;
		}

		if (frame.getVideoMode().getPixelFormat() != openni::PIXEL_FORMAT_DEPTH_1_MM && frame.getVideoMode().getPixelFormat() != openni::PIXEL_FORMAT_DEPTH_100_UM)
		{
			printf("Unexpected frame format\n");
			continue;
		}

		openni::DepthPixel* pDepth = (openni::DepthPixel*)frame.getData();
		int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;
		
		printf("[%08llu] %8d fps:%d\n", (long long)frame.getTimestamp(), pDepth[middleIndex], stream->getVideoMode().getFps());

	}

	stream->stop();
	stream->destroy();

}



