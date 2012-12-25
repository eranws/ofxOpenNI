#include "ofxDepthStream.h"

#include "OpenNI.h"


void ofxDepthStream::setup()
{
	using namespace openni;

	openni::Status rc = openni::OpenNI::initialize();
	if (rc != ONI_STATUS_OK)
	{
		throw ("Initialize failed\n%s\n", openni::OpenNI::getExtendedError());
	}

	device = new openni::Device;
	rc = device->open(ANY_DEVICE);
	if (rc != ONI_STATUS_OK)
	{
		throw ("Couldn't open device\n%s\n", OpenNI::getExtendedError());
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
	using namespace openni;
	openni::Status rc;
	
	openni::VideoStream depth;

	if (device->getSensorInfo(SENSOR_DEPTH) != NULL)
	{
		rc = depth.create(*device, SENSOR_DEPTH);
		if (rc != STATUS_OK)
		{
			throw ("Couldn't create depth stream\n%s\n", OpenNI::getExtendedError());
		}
	}

	rc = depth.start();
	if (rc != STATUS_OK)
	{
		throw ("Couldn't start the depth stream\n%s\n", OpenNI::getExtendedError());
		
	}

	VideoFrameRef frame;

	while (isThreadRunning())
	{
		rc = depth.readFrame(&frame);
		if (rc != STATUS_OK)
		{
			printf("Wait failed\n");
			continue;
		}

		if (frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_1_MM && frame.getVideoMode().getPixelFormat() != PIXEL_FORMAT_DEPTH_100_UM)
		{
			printf("Unexpected frame format\n");
			continue;
		}

		DepthPixel* pDepth = (DepthPixel*)frame.getData();
		int middleIndex = (frame.getHeight()+1)*frame.getWidth()/2;
		
		printf("[%08llu] %8d fps:%d\n", (long long)frame.getTimestamp(), pDepth[middleIndex], depth.getVideoMode().getFps());

	}

	depth.stop();
	depth.destroy();
	device->close();
	OpenNI::shutdown();

}



